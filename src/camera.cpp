#include "lens_trace/camera.h"

Camera::Camera(float positionX, float positionY, float positionZ, float yaw, float pitch, float roll) {
  this->bufferData.position[0] = positionX;
  this->bufferData.position[1] = positionY;
  this->bufferData.position[2] = positionZ;

  this->bufferData.yaw = yaw;
  this->bufferData.pitch = pitch;
  this->bufferData.roll = roll;

  this->bufferData.frameCount = 0;
}

Camera::~Camera() {

}

float Camera::getPositionX() {
  return this->bufferData.position[0];
}

float Camera::getPositionY() {
  return this->bufferData.position[1];
}

float Camera::getPositionZ() {
  return this->bufferData.position[2];
}

float Camera::getYaw() {
  return this->bufferData.yaw;
}

float Camera::getPitch() {
  return this->bufferData.pitch;
}

float Camera::getRoll() {
  return this->bufferData.roll;
}

uint32_t Camera::getFrameCount() {
  return this->bufferData.frameCount;
}

void Camera::setPosition(float x, float y, float z) {
  this->bufferData.position[0] = x;
  this->bufferData.position[1] = y;
  this->bufferData.position[2] = z;
}

void Camera::updatePosition(float x, float y, float z) {
  this->bufferData.position[0] += x;
  this->bufferData.position[1] += y;
  this->bufferData.position[2] += z;
}

void Camera::setRotation(float yaw, float pitch, float roll) {
  this->bufferData.yaw = yaw;
  this->bufferData.pitch = pitch;
  this->bufferData.roll = roll;
}

void Camera::updateRotation(float yaw, float pitch, float roll) {
  this->bufferData.yaw += yaw;
  this->bufferData.pitch += pitch;
  this->bufferData.roll += roll;
}

void Camera::incrementFrameCount() {
  this->bufferData.frameCount += 1;
}

void Camera::resetFrameCount() {
  this->bufferData.frameCount = 0;
}

void* Camera::getCameraBuffer() {
  return &this->bufferData;
}

uint64_t Camera::getCameraBufferSize() {
  return sizeof(BufferData);
}