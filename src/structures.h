#pragma once

enum StructureType {
  STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
  STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL
};

enum KernelMode {
  KERNEL_MODE_LINEAR,
  KERNEL_MODE_TILE
};

enum ThreadOrganizationMode {
  THREAD_ORGANIZATION_MODE_MAX_FIT,
  THREAD_ORGANIZATION_MODE_CUSTOM
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
};