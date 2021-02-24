#pragma once
#define CL_TARGET_OPENCL_VERSION 300
#include "renderer.h"

#include <stdio.h>
#include <time.h>

#include <CL/cl.h>

enum KernelMode {
  KERNEL_MODE_LINEAR,
  KERNEL_MODE_TILE
};

struct RenderPropertiesOpenCL {
  StructureType sType;
  void* pNext;
  KernelMode kernelMode;
  uint64_t imageWidth;
  uint64_t imageHeight;
  uint64_t imageDepth;
};

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
public:
  RendererOpenCL();
  ~RendererOpenCL();

  void render(void* pNext);
};