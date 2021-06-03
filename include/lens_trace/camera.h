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
  uint32_t frameCount;

  void* cameraBuffer;
public:
  Camera(float positionX, float positionY, float positionZ, float yaw = 0, float pitch = 0, float roll = 0);
  ~Camera();

  float getPositionX();
  float getPositionY();
  float getPositionZ();
  float getYaw();
  float getPitch();
  float getRoll();
  uint32_t getFrameCount();

  void setPosition(float x, float y, float z);
  void updatePosition(float x, float y, float z);

  void setRotation(float yaw, float pitch, float roll);
  void updateRotation(float yaw, float pitch, float roll);

  void incrementFrameCount();
  void resetFrameCount();

  void* getCameraBuffer();
  uint64_t getCameraBufferSize();
};