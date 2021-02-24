#pragma once
#include <stdint.h>

class Renderer {
protected:
  uint64_t imageWidth;
  uint64_t imageHeight;
  uint64_t imageDepth;
public:
  virtual void setResolution(uint64_t width, uint64_t height, uint64_t depth) = 0;
  virtual void render() = 0;
};