#pragma once

#include <Arduino.h>

// Ultrasonic Sensor Pins (5-pin version: VCC, TRIG, ECHO, OUT, GND)
const int TRIG_PIN = 2;
const int ECHO_PIN = 3;
// VCC to 5V, GND to GND on Arduino

// Motor Driver Pins (L298N)
const int MOTOR_A_EN = 13;  // ENA
const int MOTOR_A_IN1 = 12; // IN1
const int MOTOR_A_IN2 = 11; // IN2
const int MOTOR_B_EN = 8;   // ENB
const int MOTOR_B_IN1 = 10; // IN3
const int MOTOR_B_IN2 = 9;  // IN4

// IR Sensor Pins
const int IR_LEFT = 3;
const int IR_RIGHT = 2;

// Servo Pins
const int SERVO_PIN = 4;

// Color Sensor Pins (TCS3200 - 8 pin version)
// VCC to 5V, GND to GND
const int S0 = A0;   // Frequency scaling
const int S1 = A1;   // Frequency scaling
const int S2 = A2;   // Color filter selection
const int S3 = A3;   // Color filter selection
const int OUT = A4;  // Output frequency
const int OE = A5;   // Output Enable (active LOW)
// S4 is typically not used or tied to GND
