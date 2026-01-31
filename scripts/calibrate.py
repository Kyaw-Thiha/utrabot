#!/usr/bin/env python3
import argparse
import sys
import time

import serial


def read_line(ser):
    return ser.readline().decode("ascii", "ignore").strip()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", required=True)
    parser.add_argument("--sensor", required=True, choices=["ir", "color", "ultra"])
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--out", default=None)
    args = parser.parse_args()

    cmd_map = {"ir": "CAL_IR", "color": "CAL_COLOR", "ultra": "CAL_ULTRA"}
    out_map = {
        "ir": "cal_ir.json",
        "color": "cal_color.json",
        "ultra": "cal_ultra.json",
    }
    out_path = args.out or out_map[args.sensor]

    ser = serial.Serial(args.port, args.baud, timeout=1)
    time.sleep(2)

    ser.write((cmd_map[args.sensor] + "\n").encode("ascii"))

    while True:
        line = read_line(ser)
        if not line:
            continue

        if line.startswith("IR:READY") or line.startswith("COLOR:READY") or line.startswith("ULTRA:READY"):
            print(line)
            input("Position sensor and press Enter to continue...")
            ser.write(b"NEXT\n")
            continue

        if line.startswith("{") and line.endswith("}"):
            with open(out_path, "w", encoding="ascii") as f:
                f.write(line + "\n")
            print("Saved:", out_path)
            break

        print(line)


if __name__ == "__main__":
    sys.exit(main())
