#pragma once
#include <stdint.h>

enum StructureType {
  STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
  STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL,
  STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES,
  STRUCTURE_TYPE_BUFFER_TO_RAW_FILE_PROPERTIES,
  STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES
};

enum RenderPlatform {
  RENDER_PLATFORM_OPENCL,
  RENDER_PLATFORM_CUDA,
  RENDER_PLATFORM_VULKAN
};

enum KernelMode {
  KERNEL_MODE_LINEAR,
  KERNEL_MODE_TILE
};

enum ThreadOrganizationMode {
  THREAD_ORGANIZATION_MODE_MAX_FIT,
  THREAD_ORGANIZATION_MODE_CUSTOM
};

enum ImageType {
  IMAGE_TYPE_JPEG,
};

enum AccelerationStructureType {
  ACCELERATION_STRUCTURE_TYPE_BVH,
};

struct ThreadOrganizationOpenCL {
  StructureType sType;
  void* pNext;
  uint64_t workBlockSize[2];
  uint64_t threadGroupSize[2];
};

struct RenderPropertiesOpenCL {
  StructureType sType;
  void* pNext;
  KernelMode kernelMode;
  uint64_t imageDimensions[3];
  ThreadOrganizationMode threadOrganizationMode;
  ThreadOrganizationOpenCL* pThreadOrganization;
  void* pOutputBuffer;
  uint64_t outputBufferSize;
  void* pAccelerationStructure;
  void* pModel;
};

struct BufferToImageProperties {
  StructureType sType;
  void* pNext;
  void* pBuffer;
  uint64_t bufferSize;
  uint64_t imageDimensions[3];
  ImageType imageType;
  const char* filename;
};

struct BufferToRawFileProperties {
  StructureType sType;
  void* pNext;
  void* pBuffer;
  uint64_t bufferSize;
  uint64_t imageDimensions[3];
  const char* filename;
};

struct AccelerationStructureProperties {
  StructureType sType;
  void* pNext;
  AccelerationStructureType accelerationStructureType;
  void* pModel;
};