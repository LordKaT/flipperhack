#!/usr/bin/env python3
import sys
import os
import serial
import glob
import time


BAUD = 230400


def find_flipper_port():
    """Find the most likely Flipper serial port."""
    ports = glob.glob("/dev/serial/by-id/usb-Flipper_Devices_Inc.*_if00")
    if ports:
        return ports[0]
    ports = glob.glob("/dev/ttyACM*")
    if ports:
        return ports[0]
    print("ERROR: No Flipper serial port found")
    sys.exit(1)


def read_until_prompt(ser, timeout=1.0):
    """Drain until '>' prompt or timeout, ignoring binary noise."""
    end = time.time() + timeout
    out = b""
    while time.time() < end:
        b = ser.readline()
        if not b:
            continue
        out += b
        if b.strip().endswith(b">:"):
            break
    return out.decode(errors="ignore")


def push_binary(local_path, remote_path):
    size = os.path.getsize(local_path)
    print(f"➡  Uploading {local_path} ({size} bytes) → {remote_path}")

    port = find_flipper_port()
    print(f"📡 Using Flipper on: {port}")

    ser = serial.Serial(port, baudrate=BAUD, timeout=0.2)
    time.sleep(0.2)
    read_until_prompt(ser)

    # Ensure directory exists
    parent = "/".join(remote_path.split("/")[:-1])
    ser.write(f"storage mkdir {parent}\r\n".encode())
    ser.flush()
    read_until_prompt(ser)

    # Remove file if exists
    ser.write(f"storage remove {remote_path}\r\n".encode())
    ser.flush()
    read_until_prompt(ser)

    # --- IMPORTANT ---
    # This tells Flipper: "I am about to send EXACTLY <size> raw bytes"
    ser.write(f"storage write_chunk {remote_path} {size}\r\n".encode())
    ser.flush()

    # Small delay before raw chunk
    time.sleep(0.05)

    # Read file fully
    with open(local_path, "rb") as f:
        raw = f.read()

    # MUST be 1 single write()
    ser.write(raw)
    ser.flush()

    # Now wait for "Ready" → prompt
    result = read_until_prompt(ser, timeout=2.0)
    print(result)

    ser.close()
    print("✅ Upload complete.")


if __name__ == "__main__":
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: push_to_flipper.py LOCAL_FILE [REMOTE_PATH]")
        sys.exit(1)

    local = sys.argv[1]

    if len(sys.argv) == 3:
        remote = sys.argv[2]
    else:
        base = os.path.basename(local)
        remote = f"/ext/apps_data/flipperhack/{base}"

    push_binary(local, remote)
