#pragma once
#include "renderer_opencl.h"

enum RenderPlatform {
  RENDER_PLATFORM_OPENCL,
  RENDER_PLATFORM_CUDA,
  RENDER_PLATFORM_VULKAN
};

class Engine {
private:
  RenderPlatform renderPlatform;
  Renderer* renderer;
public:
  Engine(RenderPlatform renderPlatform);
  ~Engine();

  void render(void* pNext);
};