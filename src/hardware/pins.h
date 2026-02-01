#pragma once

#include <Arduino.h>

// Ultrasonic Sensor Pins (5-pin version: VCC, TRIG, ECHO, OUT, GND)
const int TRIG_PIN = 2;
const int ECHO_PIN = 3;
// VCC to 5V, GND to GND on Arduino

// IR Sensor Pins
const int IR_LEFT = 8;
const int IR_RIGHT = 9;

// Color Sensor Pins (TCS3200 - 8 pin version)
// VCC to 5V, GND to GND
const int S0 = A0;   // Frequency scaling
const int S1 = A1;   // Frequency scaling
const int S2 = A2;   // Color filter selection
const int S3 = A3;   // Color filter selection
const int OUT = A4;  // Output frequency
const int OE = A5;   // Output Enable (active LOW)
// S4 is typically not used or tied to GND
