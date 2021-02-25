#pragma once

enum StructureType {
  STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
  STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL,
  STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES
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
  IMAGE_TYPE_PNG
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
};

struct BufferToImageProperties {
  StructureType sType;
  void* pNext;
  void* pBuffer;
  uint64_t imageDimensions[3];
  ImageType imageType;
  const char* filename;
};