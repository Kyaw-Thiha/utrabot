#include "core/telemetry.h"
#include <Arduino.h>

namespace telemetry {
void init() { Serial.begin(115200); }
void log(const char *msg) { Serial.println(msg); }
void flush() {}
} // namespace telemetry
