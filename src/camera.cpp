#include "camera.h"

Camera::Camera(float positionX, float positionY, float positionZ, float yaw) {
  this->position[0] = positionX;
  this->position[1] = positionY;
  this->position[2] = positionZ;

  this->yaw = yaw;

  this->cameraBuffer = malloc(sizeof(float) * 4);
  memcpy((float*)this->cameraBuffer + 0, this->position, sizeof(float) * 3);
  memcpy((float*)this->cameraBuffer + 3, &this->yaw, sizeof(float) * 1);
}

Camera::~Camera() {

}

void* Camera::getCameraBuffer() {
  return this->cameraBuffer;
}

uint64_t Camera::getCameraBufferSize() {
  return sizeof(float) * 4;
}