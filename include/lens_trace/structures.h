#pragma once
#include <stdint.h>

enum StructureType {
  STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
  STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL,
  STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
  STRUCTURE_TYPE_THREAD_ORGANIZATION_CUDA,
  STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES,
  STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES
};

enum RenderPlatform {
  RENDER_PLATFORM_OPENCL,
  RENDER_PLATFORM_CUDA,
  RENDER_PLATFORM_OPTIX
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

enum AccelerationStructureExplicitType {
  ACCELERATION_STRUCTURE_TYPE_BVH,
};

struct ThreadOrganizationOpenCL {
  StructureType sType;
  void* pNext;
  uint64_t workBlockSize[2];
  uint64_t threadGroupSize[2];
};

struct ThreadOrganizationCUDA {
  StructureType sType;
  void* pNext;
  uint64_t blockSize[2];
};

struct RenderPropertiesOpenCL {
  StructureType sType;
  void* pNext;
  KernelMode kernelMode;
  ThreadOrganizationMode threadOrganizationMode;
  ThreadOrganizationOpenCL* pThreadOrganization;
  uint64_t imageDimensions[3];
  void* pOutputBuffer;
  uint64_t outputBufferSize;
  void* pAccelerationStructureExplicit;
  void* pModel;
  void* pCamera;
};

struct RenderPropertiesCUDA {
  StructureType sType;
  void* pNext;
  KernelMode kernelMode;
  ThreadOrganizationMode threadOrganizationMode;
  ThreadOrganizationCUDA* pThreadOrganization;
  uint64_t imageDimensions[3];
  void* pOutputBuffer;
  uint64_t outputBufferSize;
  void* pAccelerationStructureExplicit;
  void* pModel;
  void* pCamera;
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

struct AccelerationStructureExplicitProperties {
  StructureType sType;
  void* pNext;
  AccelerationStructureExplicitType accelerationStructureExplicitType;
  void* pModel;
};