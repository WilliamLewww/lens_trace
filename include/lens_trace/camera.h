#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <algorithm>

class Camera {
private:
  float position[3];
  float yaw;
  float pitch;
  float roll;

  void* cameraBuffer;
public:
  Camera(float positionX, float positionY, float positionZ, float yaw = 0, float pitch = 0, float roll = 0);
  ~Camera();

  void updatePosition(float x, float y, float z);

  void* getCameraBuffer();
  uint64_t getCameraBufferSize();
};