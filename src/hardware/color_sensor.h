#pragma once

namespace hardware {

struct ColorRaw {
  int r;
  int g;
  int b;
};

struct ColorCal {
  ColorRaw white;
  ColorRaw black;
};

struct ColorNorm {
  float r;
  float g;
  float b;
};

void colorInit();
ColorRaw colorReadRawRgb();

void colorSetCal(const ColorCal &cal);
ColorCal colorGetCal();

ColorNorm colorReadNormalized();
bool colorCalibrate(int white_samples, int black_samples);

} // namespace hardware
