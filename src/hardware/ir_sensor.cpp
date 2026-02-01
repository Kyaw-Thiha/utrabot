#include "hardware/ir_sensor.h"

#include <Arduino.h>

#include "hardware/pins.h"

namespace hardware {
static IrCal g_ir_cal;

// Averages multiple raw reads for a stable sample.
static int avgSamples(int (*readFn)(), int samples) {
  long sum = 0;

  for (int i = 0; i < samples; ++i)
    sum += readFn();

  return static_cast<int>(sum / samples);
}

// Clamps an integer into [lo, hi].
static int clampi(int v, int lo, int hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

// Stores calibration parameters for later reads.
void irSetCal(const IrCal &cal) { g_ir_cal = cal; }

// Returns the current calibration parameters.
IrCal irGetCal() { return g_ir_cal; }

// Sets up IR sensor pin modes.
void irInit() {
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
}

// Returns the raw left IR sensor reading.
int irReadLeftRaw() { return digitalRead(IR_LEFT); }

// Returns the raw right IR sensor reading.
int irReadRightRaw() { return digitalRead(IR_RIGHT); }

// Returns a normalized left sensor reading in 0..1000.
int irReadLeft() {
  int raw = irReadLeftRaw();
  int denom = g_ir_cal.left_white - g_ir_cal.left_black;

  if (denom == 0)
    return 0;

  int norm = (raw - g_ir_cal.left_black) * 1000 / denom;
  return clampi(norm, 0, 1000);
}

// Returns a normalized right sensor reading in 0..1000.
int irReadRight() {
  int raw = irReadRightRaw();
  int denom = g_ir_cal.right_white - g_ir_cal.right_black;

  if (denom == 0)
    return 0;

  int norm = (raw - g_ir_cal.right_black) * 1000 / denom;
  return clampi(norm, 0, 1000);
}

// Samples white and black surfaces to build thresholds and scales.
bool irCalibrate(int white_samples, int black_samples) {
  // user places sensors over white, then black
  if (white_samples <= 0 || black_samples <= 0)
    return false;

  g_ir_cal.left_white = avgSamples(irReadLeftRaw, white_samples);
  g_ir_cal.right_white = avgSamples(irReadRightRaw, white_samples);

  g_ir_cal.left_black = avgSamples(irReadLeftRaw, black_samples);
  g_ir_cal.right_black = avgSamples(irReadRightRaw, black_samples);

  g_ir_cal.left_thresh = (g_ir_cal.left_white + g_ir_cal.left_black) / 2;
  g_ir_cal.right_thresh = (g_ir_cal.right_white + g_ir_cal.right_black) / 2;

  return true;
}

} // namespace hardware
