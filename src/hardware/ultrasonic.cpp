#include "hardware/ultrasonic.h"

namespace hardware {
static UltrasonicCal g_ultra_cal;

// Sorts the buffer in ascending order for median extraction.
static void sortBuffer(int *buf, int n) {
  for (int i = 1; i < n; ++i) {
    int key = buf[i];
    int j = i - 1;
    while (j >= 0 && buf[j] > key) {
      buf[j + 1] = buf[j];
      --j;
    }
    buf[j + 1] = key;
  }
}

// Returns the median element from a small sample buffer.
static int medianOfBuffer(int *buf, int n) {
  sortBuffer(buf, n);
  return buf[n / 2];
}

// Stores calibration parameters for later reads.
void ultrasonicSetCal(const UltrasonicCal &cal) { g_ultra_cal = cal; }

// Returns the current calibration parameters.
UltrasonicCal ultrasonicGetCal() { return g_ultra_cal; }

// Reads a raw value, applies calibration, and enforces bounds.
int ultrasonicReadCm() {
  int raw = ultrasonicReadCmRaw();
  int cm = static_cast<int>(g_ultra_cal.slope * raw + g_ultra_cal.offset);

  if (cm < g_ultra_cal.min_cm || cm > g_ultra_cal.max_cm)
    return -1;

  return cm;
}

// Simple linear fit: cm = slope * raw + offset
bool ultrasonicCalibrate(const int *known_cm, int count) {
  if (count < 2)
    return false;

  // take N readings per known distance, use median to reduce noise
  const int kSamples = 15;
  long long sum_x = 0, sum_y = 0;
  long long sum_xx = 0, sum_xy = 0;

  for (int i = 0; i < count; ++i) {
    int buf[kSamples];

    for (int s = 0; s < kSamples; ++s) {
      buf[s] = ultrasonicReadCmRaw();
    }

    int raw_med = medianOfBuffer(buf, kSamples);

    sum_x += raw_med;
    sum_y += known_cm[i];
    sum_xx += 1LL * raw_med * raw_med;
    sum_xy += 1LL * raw_med * known_cm[i];
  }

  long long n = count;
  long long denom = n * sum_xx - sum_x * sum_x;

  if (denom == 0)
    return false;

  g_ultra_cal.slope = static_cast<float>(n * sum_xy - sum_x * sum_y) / denom;
  g_ultra_cal.offset =
      static_cast<float>(sum_y * sum_xx - sum_x * sum_xy) / denom;

  return true;
}

// Reads multiple raw samples and returns their median.
int ultrasonicReadRawMedian(int samples) {
  if (samples <= 0)
    return ultrasonicReadCmRaw();

  const int kMaxSamples = 20;
  int n = samples > kMaxSamples ? kMaxSamples : samples;
  int buf[kMaxSamples];

  for (int i = 0; i < n; ++i) {
    buf[i] = ultrasonicReadCmRaw();
  }

  return medianOfBuffer(buf, n);
}

// Computes and stores calibration from paired raw/known distance samples.
bool ultrasonicSetCalFromPairs(const int *raw_med, const int *known_cm,
                               int count) {
  if (count < 2)
    return false;

  long long sum_x = 0, sum_y = 0;
  long long sum_xx = 0, sum_xy = 0;

  for (int i = 0; i < count; ++i) {
    int x = raw_med[i];
    int y = known_cm[i];
    sum_x += x;
    sum_y += y;
    sum_xx += 1LL * x * x;
    sum_xy += 1LL * x * y;
  }

  long long n = count;
  long long denom = n * sum_xx - sum_x * sum_x;

  if (denom == 0)
    return false;

  g_ultra_cal.slope = static_cast<float>(n * sum_xy - sum_x * sum_y) / denom;
  g_ultra_cal.offset =
      static_cast<float>(sum_y * sum_xx - sum_x * sum_xy) / denom;
  return true;
}

} // namespace hardware
