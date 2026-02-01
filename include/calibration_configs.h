#pragma once

namespace calibration_config {

static const long kDefaultBaud = 115200;

static const int kIrSamples = 20;
static const int kColorSamples = 20;
static const int kUltraSamples = 15;

static const int kUltraPoints[] = {10, 20, 30, 40, 60, 80};
static const int kUltraPointCount =
    sizeof(kUltraPoints) / sizeof(kUltraPoints[0]);

static const int kUltraCalSamples = 15;
static const int kUltraMedianMaxSamples = 20;

} // namespace calibration_config
