#include "lens_trace/camera.h"

Camera::Camera(float positionX, float positionY, float positionZ, float yaw, float pitch, float roll) {
  this->position[0] = positionX;
  this->position[1] = positionY;
  this->position[2] = positionZ;

  this->yaw = yaw;
  this->pitch = pitch;
  this->roll = roll;

  this->frameCount = 0;

  this->cameraBuffer = malloc(sizeof(float) * 7);
  memcpy((float*)this->cameraBuffer + 0, this->position, sizeof(float) * 3);
  memcpy((float*)this->cameraBuffer + 3, &this->yaw, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 4, &this->pitch, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 5, &this->roll, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 6, &this->frameCount, sizeof(float) * 1);
}

Camera::~Camera() {
  free(this->cameraBuffer);
}

float Camera::getPositionX() {
  return this->position[0];
}

float Camera::getPositionY() {
  return this->position[1];
}

float Camera::getPositionZ() {
  return this->position[2];
}

float Camera::getYaw() {
  return this->yaw;
}

float Camera::getPitch() {
  return this->pitch;
}

float Camera::getRoll() {
  return this->roll;
}

void Camera::setPosition(float x, float y, float z) {
  this->position[0] = x;
  this->position[1] = y;
  this->position[2] = z;

  memcpy((float*)this->cameraBuffer + 0, this->position, sizeof(float) * 3);
}

void Camera::updatePosition(float x, float y, float z) {
  this->position[0] += x;
  this->position[1] += y;
  this->position[2] += z;

  memcpy((float*)this->cameraBuffer + 0, this->position, sizeof(float) * 3);
}

void Camera::setRotation(float yaw, float pitch, float roll) {
  this->yaw = yaw;
  this->pitch = pitch;
  this->roll = roll;

  memcpy((float*)this->cameraBuffer + 3, &this->yaw, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 4, &this->pitch, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 5, &this->roll, sizeof(float) * 1);
}

void Camera::updateRotation(float yaw, float pitch, float roll) {
  this->yaw += yaw;
  this->pitch += pitch;
  this->roll += roll;

  memcpy((float*)this->cameraBuffer + 3, &this->yaw, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 4, &this->pitch, sizeof(float) * 1);
  memcpy((float*)this->cameraBuffer + 5, &this->roll, sizeof(float) * 1);
}

void Camera::incrementFrameCount() {
  this->frameCount += 1;
  memcpy((float*)this->cameraBuffer + 6, &this->frameCount, sizeof(float) * 1);
}

void Camera::resetFrameCount() {
  this->frameCount = 0;
  memcpy((float*)this->cameraBuffer + 6, &this->frameCount, sizeof(float) * 1);
}

void* Camera::getCameraBuffer() {
  return this->cameraBuffer;
}

uint64_t Camera::getCameraBufferSize() {
  return sizeof(float) * 7;
}