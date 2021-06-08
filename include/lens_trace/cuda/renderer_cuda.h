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

class RendererCUDA : public Renderer {
private:
  CUdevice device;
  CUcontext context;

  std::map<std::string, nvrtcProgram> programMap;

  void compileKernel(std::string kernelFilePath);
public:
  RendererCUDA();
  ~RendererCUDA();

  void render(void* pRenderProperties);
};