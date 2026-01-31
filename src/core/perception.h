#pragma once
namespace perception {
void init();
void update();
bool isBlackDetected();
bool obstacleLeft();
bool obstacleRight();
int distanceAheadCm();
} // namespace perception
