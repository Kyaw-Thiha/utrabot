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

struct ColorTarget {
  float r;
  float g;
  float b;
};

struct ColorTargets {
  ColorTarget cardboard;
  ColorTarget black;
  ColorTarget red;
  ColorTarget blue;
  ColorTarget green;
};

void colorInit();
ColorRaw colorReadRawRgb();

void colorSetCal(const ColorCal &cal);
ColorCal colorGetCal();

ColorNorm colorReadNormalized();
bool colorCalibrate(int white_samples, int black_samples);

void colorSetTargets(const ColorTargets &targets);
ColorTargets colorGetTargets();

} // namespace hardware
