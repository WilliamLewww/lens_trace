#pragma once
#include <stdint.h>
#include <stddef.h>

enum StructureType {
  STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL
};

class Renderer {
protected:
  uint64_t imageWidth;
  uint64_t imageHeight;
  uint64_t imageDepth;
public:
  virtual void setResolution(uint64_t width, uint64_t height, uint64_t depth) = 0;
  virtual void render(void* pNext = NULL) = 0;
};