#pragma once

#include "renderer_opencl.h"
#include "renderer_cuda.h"
#include "renderer_optix.h"

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