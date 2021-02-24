#pragma once
#define CL_TARGET_OPENCL_VERSION 300
#include "renderer.h"

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
public:
  RendererOpenCL();
  ~RendererOpenCL();

  void setResolution(uint64_t width, uint64_t height, uint64_t depth);
  void render();
};