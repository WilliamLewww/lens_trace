#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"

#include "lens_trace/structures.h"
#include "lens_trace/renderer.h"
#include "lens_trace/camera.h"
#include "lens_trace/model.h"
#include "lens_trace/acceleration_structure_explicit.h"

#ifdef OPENCL_ENABLED
#include "lens_trace/opencl/renderer_opencl.h"
#endif

#ifdef CUDA_ENABLED
#include "lens_trace/cuda/renderer_cuda.h"
#endif

#ifdef OPTIX_ENABLED
#include "lens_trace/optix/renderer_optix.h"
#endif

struct RendererParsed {
  RenderPlatform renderPlatform = RENDER_PLATFORM_OPENCL;
  std::string kernelFilePath = "resources/kernels/basic.kernel";
  std::string kernelName = "basic";
  KernelMode kernelMode = KERNEL_MODE_LINEAR;
  ThreadOrganizationMode threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT;
  uint64_t workBlockSize[2] = {32, 32};
  uint64_t threadGroupSize[2] = {32, 32};
  uint64_t blockSize[2] = {32, 32};
  uint64_t imageDimensions[3] = {2048, 2048, 3};
};

struct CameraParsed {
  float position[3] = {0, 0, 0};
  float pitch = 0;
  float yaw = 0;
  float roll = 0;
};

struct ModelParsed {
  std::string filePath;
};

struct WorldParsed {
  std::vector<ModelParsed> models;
};

struct OutputParsed {
  std::string filePath = "output.jpg";
};

class SceneParser {
private:
  RendererParsed rendererParsed;
  CameraParsed cameraParsed;
  WorldParsed worldParsed;
  OutputParsed outputParsed;
public:
  SceneParser(std::string filename);
  ~SceneParser();

  uint64_t getOutputBufferSize();
  RenderPlatform getRenderPlatform();

  void* createOutputBuffer();

  Camera* createCamera();
  Model* createModel();
  AccelerationStructureExplicit* createAccelerationStructure(Model* model);

  RenderPropertiesOpenCL getRenderPropertiesOpenCL(void* outputBuffer, AccelerationStructureExplicit* accelerationStructureExplicit, Model* model, Camera* camera);
  RenderPropertiesCUDA getRenderPropertiesCUDA(void* outputBuffer, AccelerationStructureExplicit* accelerationStructureExplicit, Model* model, Camera* camera);
  BufferToImageProperties getBufferToImageProperties(void* outputBuffer);
};