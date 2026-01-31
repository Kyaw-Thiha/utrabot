#pragma once

namespace hardware {

struct UltrasonicCal {
  float slope = 1.0f;  // cm per raw unit
  float offset = 0.0f; // cm
  int min_cm = 2;
  int max_cm = 400;
  int max_spread = 5; // max acceptable spread in cm for calibration samples
};

void ultrasonicInit();
int ultrasonicReadCm();    // calibrated
int ultrasonicReadCmRaw(); // raw
void ultrasonicSetCal(const UltrasonicCal &cal);
UltrasonicCal ultrasonicGetCal();

bool ultrasonicCalibrate(const int *known_cm, int count);
int ultrasonicReadRawMedian(int samples);
bool ultrasonicSetCalFromPairs(const int *raw_med, const int *known_cm,
                               int count);

} // namespace hardware
