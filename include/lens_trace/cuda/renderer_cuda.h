#pragma once

#include "lens_trace/renderer.h"
#include "lens_trace/acceleration_structure_explicit.h"
#include "lens_trace/camera.h"
#include "lens_trace/structures.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <nvrtc.h>

#include <stdio.h>
#include <time.h>
#include <map>

struct ProgramCUDA {
  nvrtcProgram program;
  CUmodule module;
};

class RendererCUDA : public Renderer {
private:
  CUdevice device;
  CUcontext context;

  std::map<std::string, ProgramCUDA> programMap;

  void compileKernel(std::string kernelFilePath);
public:
  RendererCUDA();
  ~RendererCUDA();

  void render(void* pRenderProperties);
};