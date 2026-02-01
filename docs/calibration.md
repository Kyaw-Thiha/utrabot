# Calibration Guide

This document explains how to run sensor calibrations via USB serial and how
the calibration values are computed.

## Helpful Commands

### Calibrating
- IR profile logging:
```bash
python scripts/calibrate.py --port /dev/ttyACM0 --sensor ir_profile
```

- Color sensor:
```bash
python scripts/calibrate.py --port /dev/ttyACM0 --sensor color
```

- Ultrasonic sensor:
```bash
python scripts/calibrate.py --port /dev/ttyACM0 --sensor ultra
```

### Loading Calibration
```bash
python scripts/load_calibration.py --port /dev/ttyACM0
```

## 1) Running Calibration (CLI)

Prerequisites:
- Arduino Uno connected via USB
- Python installed
- `pyserial` installed: `pip install pyserial`

Commands:
- IR profile logging:
```bash
python scripts/calibrate.py --port /dev/ttyACM0 --sensor ir_profile
```

- Color sensor:
```bash
python scripts/calibrate.py --port /dev/ttyACM0 --sensor color
```

- Ultrasonic sensor:
```bash
python scripts/calibrate.py --port /dev/ttyACM0 --sensor ultra
```

Port name examples:
- Linux: `/dev/ttyACM0` or `/dev/ttyUSB0`
- macOS: `/dev/tty.usbmodem*` or `/dev/tty.usbserial*`
- Windows: `COM3` (format `COM#`)

What you will see:
- The Arduino prints `READY_*` prompts in the terminal.
- When prompted, position the sensor (white/black target or distance marker)
  and press Enter.
- The script writes a single JSON line to:
  - `cal_ir_profile.json`
  - `cal_color.json`
  - `cal_ultra.json`

Serial commands (manual use):
- `CAL_IR_PROFILE`, `CAL_COLOR`, `CAL_ULTRA` to start
- `NEXT` to advance each step
- `HELP` to list commands

Example manual session:
- Send `CAL_IR`
- Wait for `IR:READY_WHITE`, place on white, send `NEXT`
- Wait for `IR:READY_BLACK`, place on black, send `NEXT`
- JSON is printed

## 2) How Calibration Works (Overview)

IR sensor:
- Calibration is disabled for digital IR sensors; use direct reads.
- Optional: IR profile logging collects raw readings for
  cardboard, red, blue, and green surfaces.

Color sensor:
- Samples `kColorSamples` on a white reference and then a black reference.
- Stores per-channel white/black RGB.
- Normalized values are computed per channel:
  `(raw - black) / (white - black)` and clamped to 0..1.
- After normalization, captures target colors in order:
  cardboard, red, blue, green.
- Target black is derived from the normalization black reference.

Ultrasonic sensor:
- Uses known distances (10, 20, 30, 40, 60, 80 cm by default).
- For each point, collects `kUltraSamples` and uses the median raw value.
- Fits a linear model: `cm = slope * raw + offset`.
- The calibrated read clamps values outside min/max distance.
- If no calibration is loaded, it uses the default conversion
  `cm = time_us * 0.034 / 2`.

Notes:
- Use a flat target for ultrasonic.
- Keep lighting consistent for color calibration.
- Repeat calibration if environment changes.

## 3) Main Calibration Code References

Serial command handler entry points:
```cpp
// src/main.cpp
calibration_serial::init(115200);
calibration_serial::poll();
```

Command handling and prompts:
```cpp
// src/core/system/calibration_serial.cpp
// Commands: CAL_IR, CAL_IR_PROFILE, CAL_COLOR, CAL_ULTRA, NEXT
```

Load calibration values from JSON (host -> Arduino):
- Run: `python3 scripts/load_calibration.py --port /dev/ttyACM0`
- The script reads `cal_ir.json`, `cal_color.json`, `cal_ultra.json` if present.
- It sends `SET_IR`, `SET_COLOR`, `SET_COLOR_TARGETS`, and `SET_ULTRA`
  commands over Serial.

Calibration math functions:
```cpp
// src/hardware/ir_sensor.cpp
bool irCalibrate(int white_samples, int black_samples);

// src/hardware/color_sensor.cpp
bool colorCalibrate(int white_samples, int black_samples);

// src/hardware/ultrasonic.cpp
bool ultrasonicSetCalFromPairs(const int *raw_med, const int *known_cm, int count);
```
