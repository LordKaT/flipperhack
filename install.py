#!/usr/bin/env python3
import argparse
import glob
import os
import sys
import time
from pathlib import Path

import serial

BAUD = 230400
DEBUG = False  # Set to False to disable debug logs

# Accept multiple Flipper prompt variants.
PROMPT_ENDINGS = (
    b">",
    b">:",
    b">: ",
)

CHUNK_SIZE = 256


def log(msg):
    if DEBUG:
        print(f"[DEBUG] {msg}")


def log_raw(prefix, data):
    if DEBUG:
        try:
            print(f"[DEBUG] {prefix}: {data}")
        except Exception:
            print(f"[DEBUG] {prefix}: <binary data len={len(data)}>")


def find_flipper_port() -> str:
    """Auto-detect Flipper Zero USB serial port."""
    by_id = glob.glob("/dev/serial/by-id/usb-Flipper_Devices_Inc.*_if00")
    if by_id:
        return by_id[0]

    acm = glob.glob("/dev/ttyACM*")
    if acm:
        return acm[0]

    print("ERROR: Could not find Flipper Zero serial port.")
    sys.exit(1)


def read_until_prompt(ser, timeout=5.0):
    """
    Reads until the Flipper CLI prompt appears.
    """
    start = time.time()
    buf = b""
    
    log(f"Waiting for prompt (timeout={timeout}s)...")

    while time.time() - start < timeout:
        if ser.in_waiting > 0:
            chunk = ser.read(ser.in_waiting or 1)
            if chunk:
                log_raw("RECV", chunk)
                buf += chunk
                # Check if any prompt variant is IN the buffer, not just at the end
                for p in PROMPT_ENDINGS:
                    if p.decode("utf-8") in buf.decode("utf-8", errors="ignore"):
                        log("Prompt detected.")
                        return buf.decode("utf-8", errors="ignore")
        else:
            time.sleep(0.01)

    log("Timeout waiting for prompt.")
    return buf.decode("utf-8", errors="ignore")


def wake_cli(ser):
    """Force Flipper CLI to wake up and show prompt."""
    log("Waking CLI...")
    for _ in range(3):
        ser.write(b"\r\n")
        ser.flush()
        time.sleep(0.1)
    return read_until_prompt(ser, timeout=2.0)


def flipper_cmd(ser, cmd, quiet=False):
    """Send a Flipper CLI command."""
    if not quiet:
        print(f"▶ {cmd}")
    
    log(f"Sending command: {cmd}")
    ser.write(cmd.encode("ascii") + b"\r\n")
    ser.flush()
    out = read_until_prompt(ser)

    if not quiet:
        for line in out.splitlines():
            if line.strip():
                print("  ", line)

    return out


def ensure_dir(ser, path):
    flipper_cmd(ser, f"storage mkdir {path}", quiet=True)


def remove_path(ser, path):
    flipper_cmd(ser, f"storage remove {path}", quiet=True)


def read_until(ser, token, timeout=5.0):
    """Read from serial until a specific token is found."""
    start = time.time()
    buf = b""
    log(f"Waiting for '{token}' (timeout={timeout}s)...")
    while time.time() - start < timeout:
        if ser.in_waiting > 0:
            chunk = ser.read(ser.in_waiting or 1)
            if chunk:
                log_raw("RECV", chunk)
                buf += chunk
                if token.encode("utf-8") in buf:
                    return buf.decode("utf-8", errors="ignore")
        else:
            time.sleep(0.01)
    log(f"Timeout waiting for '{token}'")
    return buf.decode("utf-8", errors="ignore")

def write_file(ser, local_path: Path, remote_path: str):
    size = local_path.stat().st_size
    print(f"⬆ Uploading {local_path} → {remote_path} ({size} bytes)")

    # Always remove first so the write is clean (write_chunk appends)
    remove_path(ser, remote_path)

    with local_path.open("rb") as f:
        while True:
            chunk = f.read(CHUNK_SIZE)
            if not chunk:
                break
            
            # Command: storage write_chunk <path> <len>
            # Use \r only to avoid leaving \n in the buffer which would be read as data
            cmd = f"storage write_chunk {remote_path} {len(chunk)}"
            log(f"Sending chunk cmd: {cmd}")
            
            ser.reset_input_buffer() # Clear any pending input
            # Send a leading CR to clear any partial line, then the command, then CR
            ser.write(b"\r" + cmd.encode("ascii") + b"\r")
            ser.flush()
            
            # Wait for "Ready"
            out = read_until(ser, "Ready")
            if "Ready" not in out:
                print(f"❌ Error: Flipper did not send 'Ready'. Got: {out}")
                sys.exit(1)
            
            # Send data
            log(f"Sending {len(chunk)} bytes of data...")
            ser.write(chunk)
            ser.flush()
            
            # Now wait for prompt
            out = read_until_prompt(ser)
            if "Storage error" in out:
                print(f"❌ Error writing chunk: {out}")
                sys.exit(1)


def install_app(ser, dist_dir: Path, category: str, appid: str):
    """Replicates ufbt-launch external-install behavior."""

    dist_dir = dist_dir.resolve()

    # Pick the fap
    fap_path = dist_dir / f"{appid}.fap"
    if not fap_path.is_file():
        # fallback: grab any .fap in dist
        faps = list(dist_dir.glob("*.fap"))
        if not faps:
            print("ERROR: No .fap found in dist/")
            sys.exit(1)
        fap_path = faps[0]
        print(f"⚠ Using {fap_path.name} (appid mismatch?)")

    remote_fap = f"/ext/apps/{category}/{appid}.fap"

    # Create /ext/apps/<Category>
    ensure_dir(ser, f"/ext/apps/{category}")

    # Upload the .fap
    write_file(ser, fap_path, remote_fap)

    # Handle optional app data
    data_dir = dist_dir / "data"
    if data_dir.is_dir():
        remote_data_root = f"/ext/apps_data/{appid}"
        ensure_dir(ser, remote_data_root)

        for root, dirs, files in os.walk(data_dir):
            rel = os.path.relpath(root, data_dir)
            rel = "" if rel == "." else rel
            remote_dir = (
                remote_data_root if not rel else f"{remote_data_root}/{rel}"
            )
            ensure_dir(ser, remote_dir)
            for name in files:
                local = Path(root) / name
                remote = f"{remote_dir}/{name}"
                write_file(ser, local, remote)

    print("✅ Install complete")


def main():
    parser = argparse.ArgumentParser(description="Install Flipper app like ufbt launch.")
    parser.add_argument(
        "-s",
        "--source-dir",
        type=Path,
        default=Path("dist"),
        help="Directory containing the built .fap and data/",
    )
    parser.add_argument(
        "-c",
        "--category",
        default="Games",
        help="Apps menu category under /ext/apps/",
    )
    parser.add_argument(
        "-a",
        "--appid",
        default="flipperhack",
        help="App ID (basename of the .fap file)",
    )
    parser.add_argument(
        "-L",
        "--launch",
        action="store_true",
        help="Launch the app after install",
    )
    parser.add_argument(
        "--file",
        type=Path,
        help="Upload a specific file (debug mode)",
    )
    parser.add_argument(
        "--verify",
        action="store_true",
        help="Verify MD5 after upload",
    )
    parser.add_argument(
        "--dump",
        action="store_true",
        help="Dump remote file content (hex) after upload",
    )
    parser.add_argument(
        "--remote-path",
        help="Remote path for --file",
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="Enable debug logging",
    )

    args = parser.parse_args()
    
    global DEBUG
    if args.debug:
        DEBUG = True

    port = find_flipper_port()
    print(f"🔌 Connecting to Flipper on {port} @ {BAUD} baud…")

    ser = serial.Serial(port, BAUD, timeout=0.5)
    time.sleep(0.1)

    # Wake the shell
    wake_cli(ser)

    try:
        if args.file:
            if not args.remote_path:
                print("❌ --remote-path required with --file")
                sys.exit(1)
            write_file(ser, args.file, args.remote_path)
            
            if args.verify:
                cmd = f"storage md5 {args.remote_path}"
                out = flipper_cmd(ser, cmd)
                print(f"MD5: {out}")
            
            if args.dump:
                cmd = f"storage read {args.remote_path}"
                out = flipper_cmd(ser, cmd)
                print(f"Content:\n{out}")
                
        else:
            install_app(ser, args.source_dir, args.category, args.appid)

            if args.launch:
                print("🚀 Launching…")
                full_path = f"/ext/apps/{args.category}/{args.appid}.fap"
                flipper_cmd(ser, f"loader open {full_path}")

    finally:
        ser.close()
        print("🔌 Serial connection closed.")


if __name__ == "__main__":
    main()
