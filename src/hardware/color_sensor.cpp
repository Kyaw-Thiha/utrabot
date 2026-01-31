#include "hardware/color_sensor.h"

namespace hardware {
static ColorCal g_color_cal;
static ColorTargets g_color_targets;

// Averages multiple raw RGB reads into one sample.
static ColorRaw avgSamples(int samples) {
  long sum_r = 0, sum_g = 0, sum_b = 0;

  for (int i = 0; i < samples; ++i) {
    ColorRaw c = colorReadRawRgb();
    sum_r += c.r;
    sum_g += c.g;
    sum_b += c.b;
  }

  ColorRaw out;
  out.r = static_cast<int>(sum_r / samples);
  out.g = static_cast<int>(sum_g / samples);
  out.b = static_cast<int>(sum_b / samples);

  return out;
}

// Normalizes a raw RGB sample using current white/black calibration.
static ColorNorm colorNormalize(const ColorRaw &raw) {
  ColorNorm n;

  int dr = g_color_cal.white.r - g_color_cal.black.r;
  int dg = g_color_cal.white.g - g_color_cal.black.g;
  int db = g_color_cal.white.b - g_color_cal.black.b;

  n.r = dr ? float(raw.r - g_color_cal.black.r) / dr : 0.0f;
  n.g = dg ? float(raw.g - g_color_cal.black.g) / dg : 0.0f;
  n.b = db ? float(raw.b - g_color_cal.black.b) / db : 0.0f;

  if (n.r < 0)
    n.r = 0;
  if (n.r > 1)
    n.r = 1;
  if (n.g < 0)
    n.g = 0;
  if (n.g > 1)
    n.g = 1;
  if (n.b < 0)
    n.b = 0;
  if (n.b > 1)
    n.b = 1;

  return n;
}

// Stores calibration parameters for later reads.
void colorSetCal(const ColorCal &cal) { g_color_cal = cal; }

// Returns the current calibration parameters.
ColorCal colorGetCal() { return g_color_cal; }

// Reads raw RGB, normalizes with white/black reference, and clamps to 0..1.
ColorNorm colorReadNormalized() {
  ColorRaw raw = colorReadRawRgb();
  return colorNormalize(raw);
}

// Captures white and black references to support normalization.
bool colorCalibrate(int white_samples, int black_samples) {
  if (white_samples <= 0 || black_samples <= 0)
    return false;

  g_color_cal.white = avgSamples(white_samples);
  g_color_cal.black = avgSamples(black_samples);

  return true;
}

// Stores target color references for classification.
void colorSetTargets(const ColorTargets &targets) { g_color_targets = targets; }

// Returns the current target color references.
ColorTargets colorGetTargets() { return g_color_targets; }

} // namespace hardware
