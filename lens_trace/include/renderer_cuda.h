#ifdef CUDA_ENABLED

#pragma once

#include "renderer.h"
#include "acceleration_structure_explicit.h"
#include "camera.h"
#include "structures.h"

#include <stdio.h>
#include <time.h>

class RendererCUDA : public Renderer {
private:
public:
  RendererCUDA();
  ~RendererCUDA();

  void render(void* pRenderProperties);
};

#endif