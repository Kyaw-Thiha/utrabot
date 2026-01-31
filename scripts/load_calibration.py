#!/usr/bin/env python3
import argparse
import json
import os
import sys
import time

import serial


def read_line(ser):
    return ser.readline().decode("ascii", "ignore").strip()


def send_and_wait(ser, cmd, ok_prefix):
    ser.write((cmd + "\n").encode("ascii"))
    while True:
        line = read_line(ser)
        if not line:
            continue
        if line.startswith(ok_prefix):
            return True
        if line.startswith("ERR:"):
            raise RuntimeError(line)


def load_ir(path):
    with open(path, "r", encoding="ascii") as f:
        data = json.load(f)
    return "SET_IR {left_white} {left_black} {right_white} {right_black} {left_thresh} {right_thresh}".format(
        **data
    )


def load_color(path):
  with open(path, "r", encoding="ascii") as f:
    data = json.load(f)
  return "SET_COLOR {white_r} {white_g} {white_b} {black_r} {black_g} {black_b}".format(
    **data
  )

def load_color_targets(path):
  with open(path, "r", encoding="ascii") as f:
    data = json.load(f)
  required = [
    "cardboard_r",
    "cardboard_g",
    "cardboard_b",
    "target_black_r",
    "target_black_g",
    "target_black_b",
    "red_r",
    "red_g",
    "red_b",
    "blue_r",
    "blue_g",
    "blue_b",
    "green_r",
    "green_g",
    "green_b",
  ]
  for key in required:
    if key not in data:
      return None
  return (
    "SET_COLOR_TARGETS "
    "{cardboard_r} {cardboard_g} {cardboard_b} "
    "{target_black_r} {target_black_g} {target_black_b} "
    "{red_r} {red_g} {red_b} "
    "{blue_r} {blue_g} {blue_b} "
    "{green_r} {green_g} {green_b}"
  ).format(**data)


def load_ultra(path):
    with open(path, "r", encoding="ascii") as f:
        data = json.load(f)
    return "SET_ULTRA {slope} {offset} {min_cm} {max_cm}".format(**data)


def maybe_send(ser, path, build_cmd, ok_prefix):
    if not os.path.exists(path):
        return False
    cmd = build_cmd(path)
    send_and_wait(ser, cmd, ok_prefix)
    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--ir", default="cal_ir.json")
    parser.add_argument("--color", default="cal_color.json")
    parser.add_argument("--ultra", default="cal_ultra.json")
    args = parser.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=1)
    time.sleep(2)

    sent_any = False
  sent_any |= maybe_send(ser, args.ir, load_ir, "OK:SET_IR")
  sent_any |= maybe_send(ser, args.color, load_color, "OK:SET_COLOR")
  if os.path.exists(args.color):
    cmd = load_color_targets(args.color)
    if cmd:
      send_and_wait(ser, cmd, "OK:SET_COLOR_TARGETS")
      sent_any = True
  sent_any |= maybe_send(ser, args.ultra, load_ultra, "OK:SET_ULTRA")

    if not sent_any:
        print("No calibration files found.")
        return 1

    print("Calibration values applied.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
