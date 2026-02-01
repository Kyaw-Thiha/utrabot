# Udev Rules for Arduino DFU (Linux)

These steps allow `dfu-util` (used by PlatformIO) to access Arduino boards
without running as root.

1) Download the PlatformIO udev rules file:
```
sudo curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/scripts/99-platformio-udev.rules -o /etc/udev/rules.d/99-platformio-udev.rules
```

If `curl` is not available, use `wget`:
```
sudo wget -O /etc/udev/rules.d/99-platformio-udev.rules https://raw.githubusercontent.com/platformio/platformio-core/develop/scripts/99-platformio-udev.rules
```

2) Reload udev rules and replug the board:
```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

3) (Optional) Add your user to `plugdev` and re-login:
```
sudo usermod -a -G plugdev $USER
```

4) Put the Uno R4 Minima into DFU mode (double-tap RESET, LED pulses), then:
```
pio run -t upload
```
