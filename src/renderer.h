#pragma once
#include <stdint.h>
#include <stddef.h>

enum StructureType {
  STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL
};

class Renderer {
protected:
public:
  virtual void render(void* pNext) = 0;
};