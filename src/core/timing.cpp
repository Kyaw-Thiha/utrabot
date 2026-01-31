#include "core/timing.h"
#include <Arduino.h>

namespace timing {
static unsigned long now = 0;

void init() { now = millis(); }
void update() { now = millis(); }
unsigned long millisNow() { return now; }
} // namespace timing
