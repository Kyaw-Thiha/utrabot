#include <Arduino.h>

#include "core/system/calibration_serial.h"
#include "hardware/color_sensor.h"
#include "hardware/ir_sensor.h"
#include "hardware/ultrasonic.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  hardware::ultrasonicInit();
  hardware::irInit();
  hardware::colorInit();
  calibration_serial::init(115200);
}

void loop() {
  calibration_serial::poll();

  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
