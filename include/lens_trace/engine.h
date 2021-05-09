#pragma once

#include "lens_trace/opencl/renderer_opencl.h"
#include "lens_trace/cuda/renderer_cuda.h"
#include "lens_trace/optix/renderer_optix.h"
#include "lens_trace/renderer.h"
#include "lens_trace/structures.h"

#include "stb/stb_image_write.h"

#include <string>

class Engine {
private:
  RenderPlatform renderPlatform;
  Renderer* pRenderer;
public:
  Engine(RenderPlatform renderPlatform);
  ~Engine();

  void render(void* pRenderProperties);
  void writeBufferToImage(BufferToImageProperties bufferToImageProperties);
};