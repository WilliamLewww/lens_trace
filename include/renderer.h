#pragma once
#include <stdint.h>
#include <stddef.h>

class Renderer {
protected:
public:
  virtual void render(void* pRenderProperties) = 0;
};