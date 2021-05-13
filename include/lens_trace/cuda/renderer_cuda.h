#pragma once

#include "lens_trace/renderer.h"
#include "lens_trace/acceleration_structure_explicit.h"
#include "lens_trace/camera.h"
#include "lens_trace/structures.h"

#include <stdio.h>
#include <time.h>
#include <map>

#define KERNEL_ARGUMENTS void*, uint64_t, void*, uint64_t, void*, uint64_t, void*, uint64_t, void*, uint64_t[3], uint64_t[2], KernelMode

class RendererCUDA : public Renderer {
private:
  static std::map<std::string, void (*)(KERNEL_ARGUMENTS)> kernelMap;
public:
  RendererCUDA();
  ~RendererCUDA();

  void render(void* pRenderProperties);
};