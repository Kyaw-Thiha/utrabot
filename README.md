# utrabot

Starter PlatformIO project for Arduino Uno.

## Setup

1. activate the env
2. pip install requirements.txt

## Build

```bash
pio run
```

## Generate compile_commands.json 
```bash
pio run -t compiledb
```

## Upload

```bash
pio run -t upload
```

## Monitor

```bash
pio device monitor
```

## Components
### IR Sensor
2 of MH IR Sensor

[Product Link](https://einstronic.com/product/infrared-obstacle-sensor-module/)

### Color Sensor
Color Sensor v575

Couldn't find link


### Ultrasonic Sensor
HY-SRF05

- [Product Link](https://www.pishop.ca/product/arduino-compatible-ultrasonic-distance-measuring-sensor-module/)
- [Datasheet (Short)](https://www.micros.com.pl/mediaserver/M_HY-SRF05_0003.pdf)

### Servo Motor
Tower Pro Micro Servo 9g (SG90)

[Product Link](https://www.pishop.ca/product/sg90-180-degrees-9g-micro-servo-motor-tower-pro/)
```
    Weight : 9 g
    Size : 22 x 11.5 x 27 mm
    Operating Speed (4.8V no load): 0.12sec/60 degrees
    Stall Torque (4.8V): 17.5oz/in (1.2 kg/cm)
    Temperature Range: -30 to +60 Degree C
    Dead Band Width: 7usec Operating Voltage:3.0-7.2 Volts
```

### Motor

### Microcontroller
Arduino Uno

### Components List
- 1 medium sized breadboard
- 4 9V batteries
- 2 9V battery holders
- 10 Screws
- 1 Ultrasonic Sensor
- 1 Arduino Uno (USB-C)
- 1 Arduino wire
- 2 Wheels
- 2 DC motors
- 2 DC motor holders
- 1 Screwdriver
- 2 Servo Motor
- 1 Motor Drive
- 1 Color sensor
- 2 IR Sensors
- 20 M-M, M-F, F-F Wires (20 of each type)
- 1 Laser Cut Base
- 6 joint and connector pieces for the arm and claw

## Challenges
### Section 1: Target Shooting
The robot climbs either a straight ramp or a curved ramp (the curved ramp is worth more points).

At the top, the robot will be stopped and randomly placed somewhere on the target, from which the robot must then use color cues to navigate toward the center of the target area (black) where there is a ball waiting for them.

The team may reupload code after reacing this black section.

See Figure 2 for specific dimensions and colors.

At the center (the black zone), there is a ball.

The robot must shoot or launch this ball forward.

The team will gain maximum points if the ball is in the blue zone without touching the walls, with points decreasing the further it is from the blue zone.

If the ball touches or bounces off the wall, the team will lose points.

After shooting, the robot returns down a ramp and may collect the second battery to unlock the other section if desired.

The robots ability to navigate to the center and shoot the bar as far as possible will all count for points.

### Section 2: Obstacle Course
The robot navigates a winding path filled with sharp turns and obstacles.

The goal is to complete the course as quickly and smoothly as possible.

The path leads back to the main area, where the robot can attempt the other section or finish the run.

The faster the robot can complete the obstacle course, the more points awarded.

To complete the challenge, the robot must return to its original starting position.

Teams may choose to attempt either section, both, or neither, points are awarded based on difficulty and performance.
