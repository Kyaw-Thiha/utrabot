# Arduino IDE Setup

This project is organized for PlatformIO, but you can build/upload from the Arduino IDE by generating a sketch folder.

## Steps

1. Run the sync script from the repo root:
   ```bash
   ./scripts/sync_arduino.sh
   ```
2. Open `arduino/utrabot/utrabot.ino` in the Arduino IDE.
3. Select **Tools > Board > Arduino Uno**.
4. Select the correct **Tools > Port** for your board.
5. Open the Serial Monitor and set the baud rate to **115200**.

## Notes

- Re-run the sync script any time you change files under `src/`.
