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

  void* cameraBuffer;
public:
  Camera(float positionX, float positionY, float positionZ, float yaw);
  ~Camera();

  void* getCameraBuffer();
  uint64_t getCameraBufferSize();
};