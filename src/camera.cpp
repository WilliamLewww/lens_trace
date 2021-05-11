#include "lens_trace/camera.h"

Camera::Camera(float positionX, float positionY, float positionZ, float yaw, float pitch, float roll) {
  this->position[0] = positionX;
  this->position[1] = positionY;
  this->position[2] = positionZ;

  this->yaw = yaw;
  this->pitch = pitch;
  this->roll = roll;

  this->cameraBuffer = malloc(sizeof(float) * 6);
  memcpy((float*)this->cameraBuffer + 0, this->position, sizeof(float) * 3);
  memcpy((float*)this->cameraBuffer + 3, &this->yaw, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 4, &this->pitch, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 5, &this->roll, sizeof(float) * 1);
}

Camera::~Camera() {
  free(this->cameraBuffer);
}

void Camera::updatePosition(float x, float y, float z) {
  this->position[0] += x;
  this->position[1] += y;
  this->position[2] += z;

  memcpy((float*)this->cameraBuffer + 0, this->position, sizeof(float) * 3);
}

void* Camera::getCameraBuffer() {
  return this->cameraBuffer;
}

uint64_t Camera::getCameraBufferSize() {
  return sizeof(float) * 6;
}