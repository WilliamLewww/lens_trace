#pragma once
#define CL_TARGET_OPENCL_VERSION 300
#include "renderer.h"
#include "acceleration_structure.h"
#include "structures.h"

#include <stdio.h>
#include <time.h>

#include <CL/cl.h>

class RendererOpenCL : public Renderer {
private:
  cl_uint platformCount;
  cl_uint deviceCount;

  cl_platform_id platformID;
  cl_device_id deviceID;

  uint64_t maxWorkItemSizes[3];
  uint64_t maxWorkGroupSize;

  cl_context_properties contextProperties[3];
  cl_context context;
  cl_command_queue commandQueue;
  cl_program program;

  cl_kernel kernel;
  uint64_t workBlockSize[2];
  uint64_t threadGroupSize[2];
  uint64_t workBlockCount;
public:
  RendererOpenCL();
  ~RendererOpenCL();

  void render(void* pRenderProperties);
};