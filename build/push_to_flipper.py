#!/usr/bin/env python3
import os
import sys
import glob
import time
import base64
import serial

BAUD = 230400
TARGET_DIR = "/ext/apps_data/flipperhack"


def find_flipper_port():
    candidates = glob.glob("/dev/serial/by-id/usb-Flipper_Devices_Inc._Flipper_*_if00")
    if candidates:
        return candidates[0]
    candidates = glob.glob("/dev/ttyACM*")
    return candidates[0] if candidates else None


def read_some(ser, label="FLIPPER", timeout=0.5):
    out = b""
    t_end = time.time() + timeout
    while time.time() < t_end:
        line = ser.readline()
        if not line:
            break
        out += line
        s = line.decode("utf-8", errors="ignore").rstrip()
        if s:
            print(f"{label}: {s}")
    return out


def open_serial():
    port = find_flipper_port()
    if not port:
        print("❌ Could not find Flipper serial device.", file=sys.stderr)
        sys.exit(1)

    print(f"📡 Using Flipper on port: {port}")
    ser = serial.Serial(port, baudrate=BAUD, timeout=0.5)

    ser.write(b"\r\n")
    ser.flush()
    time.sleep(0.2)
    read_some(ser, label="BOOT", timeout=1.0)
    return ser


def send_cmd(ser, cmd, wait=True):
    print(f"CMD: {cmd}")
    ser.write((cmd + "\r\n").encode("ascii"))
    ser.flush()
    if wait:
        time.sleep(0.1)
        read_some(ser)


def storage_write_text(ser, remote_path, text_bytes):
    """Write arbitrary bytes safely using storage write + Ctrl-C, assuming only ASCII."""
    # ensure dir
    send_cmd(ser, f"storage mkdir {os.path.dirname(remote_path)}")
    # remove old
    send_cmd(ser, f"storage remove {remote_path}")

    print(f"CMD: storage write {remote_path}  (entering write mode)")
    ser.write(f"storage write {remote_path}\r\n".encode("ascii"))
    ser.flush()
    time.sleep(0.1)
    read_some(ser, label="WRITE-START")

    ser.write(text_bytes)
    ser.flush()

    # end with Ctrl-C
    ser.write(b"\x03")
    ser.flush()
    time.sleep(0.2)
    read_some(ser, label="WRITE-END", timeout=1.0)

    send_cmd(ser, f"storage stat {remote_path}")


def push_text_file(ser, local_path):
    basename = os.path.basename(local_path)
    remote_path = f"{TARGET_DIR}/{basename}"
    size = os.path.getsize(local_path)

    print(f"\n➡  TEXT {local_path}  →  {remote_path}  ({size} bytes)")

    with open(local_path, "rb") as f:
        data = f.read()
    storage_write_text(ser, remote_path, data)
    print("✅ Done.")


def push_binary_file_as_b64(ser, local_path):
    basename = os.path.basename(local_path)
    # store as .b64 alongside
    remote_path = f"{TARGET_DIR}/{basename}.b64"
    size = os.path.getsize(local_path)

    print(f"\n➡  BIN  {local_path}  →  {remote_path} (base64, from {size} bytes)")

    with open(local_path, "rb") as f:
        raw = f.read()

    # base64, single line, no newlines
    b64 = base64.b64encode(raw)  # bytes
    b64 += b"\n"  # nice to newline-terminate

    storage_write_text(ser, remote_path, b64)
    print("✅ Done (binary as base64).")


def main():
    if len(sys.argv) < 2:
        print("Usage: push_flipper_files.py <file> [file2 ...] or 'data/'", file=sys.stderr)
        sys.exit(1)

    ser = open_serial()

    paths = []
    for arg in sys.argv[1:]:
        if os.path.isdir(arg):
            for root, dirs, files in os.walk(arg):
                for name in files:
                    paths.append(os.path.join(root, name))
        else:
            paths.append(arg)

    try:
        for path in paths:
            ext = os.path.splitext(path)[1].lower()
            if ext in (".dat", ".txt", ".cfg"):
                push_text_file(ser, path)
            elif ext in (".bin",):
                push_binary_file_as_b64(ser, path)
            else:
                # default: treat as text for now
                push_text_file(ser, path)
    finally:
        ser.close()


if __name__ == "__main__":
    main()
