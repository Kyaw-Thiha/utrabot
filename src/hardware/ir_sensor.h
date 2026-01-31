#pragma once

namespace hardware {

struct IrCal {
  int left_white = 0;
  int left_black = 1023;
  int right_white = 0;
  int right_black = 1023;
  int left_thresh = 512;
  int right_thresh = 512;
  int hysteresis = 20;
};

void irInit();
int irReadLeftRaw();
int irReadRightRaw();

int irReadLeft();  // normalized 0..1000
int irReadRight(); // normalized 0..1000

void irSetCal(const IrCal &cal);
IrCal irGetCal();

bool irCalibrate(int white_samples, int black_samples);

} // namespace hardware
