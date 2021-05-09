#ifdef CUDA_ENABLED

#pragma once

#include "lens_trace/renderer.h"
#include "lens_trace/acceleration_structure_explicit.h"
#include "lens_trace/camera.h"
#include "lens_trace/structures.h"

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