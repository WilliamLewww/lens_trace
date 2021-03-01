#pragma once

#include "renderer_opencl.h"

#include "stb/stb_image_write.h"

#include <string>

class Engine {
private:
  RenderPlatform renderPlatform;
  Renderer* renderer;
public:
  Engine(RenderPlatform renderPlatform);
  ~Engine();

  void render(void* pRenderProperties);
  void writeBufferToImage(BufferToImageProperties bufferToImageProperties);
  void writeBufferToRawFile(BufferToRawFileProperties bufferToRawFileProperties);
};