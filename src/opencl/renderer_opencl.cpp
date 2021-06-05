#include "lens_trace/opencl/renderer_opencl.h"

void printKernelBuildLog(cl_device_id deviceID, cl_program program) {
  char* pPrintBuffer = (char*)malloc(4096);
  uint64_t length;
  clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, 4096, pPrintBuffer, &length);
  printf("%s\n",pPrintBuffer);
  free(pPrintBuffer);
}

RendererOpenCL::RendererOpenCL() {
  clGetPlatformIDs(1, &this->platformID, &this->platformCount);
  clGetDeviceIDs(this->platformID, CL_DEVICE_TYPE_GPU, 1, &this->deviceID, &this->deviceCount);

  clGetDeviceInfo(this->deviceID, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(this->maxWorkItemSizes), &this->maxWorkItemSizes, NULL);
  clGetDeviceInfo(this->deviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(this->maxWorkGroupSize), &this->maxWorkGroupSize, NULL);

  this->contextProperties[0] = CL_CONTEXT_PLATFORM;
  this->contextProperties[1] = (cl_context_properties)this->platformID;
  this->contextProperties[2] = 0;

  this->context = clCreateContext(this->contextProperties, 1, &this->deviceID, NULL, NULL, NULL);
  this->commandQueue = clCreateCommandQueueWithProperties(this->context, this->deviceID, NULL, NULL);
}

RendererOpenCL::~RendererOpenCL() {
  for (auto& x : this->programMap) {
    clReleaseProgram(x.second);
  }

  clReleaseCommandQueue(this->commandQueue);
  clReleaseContext(this->context);
}

void RendererOpenCL::compileKernel(std::string kernelFilePath) {
  std::string basicKernelFileName = Resource::findResource(kernelFilePath.c_str());
  FILE* pKernelFile = fopen(basicKernelFileName.c_str(), "rb");
  fseek(pKernelFile, 0, SEEK_END);
  uint32_t kernelFileSize = ftell(pKernelFile);
  fseek(pKernelFile, 0, SEEK_SET);

  char* pKernelFileBuffer = (char*)malloc(kernelFileSize + 1);
  fread(pKernelFileBuffer, 1, kernelFileSize, pKernelFile);
  fclose(pKernelFile);
  pKernelFileBuffer[kernelFileSize] = '\0';

  this->programMap[kernelFilePath] = clCreateProgramWithSource(this->context, 1, (const char**)&pKernelFileBuffer, NULL, NULL);
  free(pKernelFileBuffer);

  int error = clBuildProgram(this->programMap[kernelFilePath], 0, NULL, NULL, NULL, NULL);
  if (error != CL_SUCCESS) {
    printKernelBuildLog(this->deviceID, this->programMap[kernelFilePath]);
  }
}

void RendererOpenCL::render(void* pRenderProperties) {
  RenderPropertiesOpenCL* pRenderPropertiesOpenCL = (RenderPropertiesOpenCL*)pRenderProperties;

  if (pRenderPropertiesOpenCL->sType != STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL) {
    printf("ERROR: RenderPropertiesOpenCL sType\n");
  }

  if (this->programMap.find(pRenderPropertiesOpenCL->kernelFilePath) == this->programMap.end()) {
    compileKernel(pRenderPropertiesOpenCL->kernelFilePath);
  }
  cl_program program = this->programMap[pRenderPropertiesOpenCL->kernelFilePath];

  AccelerationStructureExplicit* pAccelerationStructureExplicit = (AccelerationStructureExplicit*)pRenderPropertiesOpenCL->pAccelerationStructureExplicit;
  Model* pModel = (Model*)pRenderPropertiesOpenCL->pModel;
  Camera* pCamera = (Camera*)pRenderPropertiesOpenCL->pCamera;

  if (pRenderPropertiesOpenCL->kernelMode == KERNEL_MODE_LINEAR) {
    this->kernel = clCreateKernel(program, "linearKernel", NULL);
  }
  if (pRenderPropertiesOpenCL->kernelMode == KERNEL_MODE_TILE) {
    this->kernel = clCreateKernel(program, "tileKernel", NULL);
  }
  if (pRenderPropertiesOpenCL->threadOrganizationMode == THREAD_ORGANIZATION_MODE_MAX_FIT) {
    uint64_t maxWorkGroupSizeKernel;
    clGetKernelWorkGroupInfo(this->kernel, this->deviceID, CL_KERNEL_WORK_GROUP_SIZE, sizeof(maxWorkGroupSizeKernel), &maxWorkGroupSizeKernel, NULL);
    
    this->workBlockSize[0] = std::min(this->maxWorkItemSizes[0], pRenderPropertiesOpenCL->imageDimensions[0]);
    this->workBlockSize[1] = std::min(this->maxWorkItemSizes[1], pRenderPropertiesOpenCL->imageDimensions[1]);

    this->threadGroupSize[0] = 32;
    this->threadGroupSize[1] = (std::min(this->maxWorkGroupSize, maxWorkGroupSizeKernel) / 32);

    this->workBlockCount = (pRenderPropertiesOpenCL->imageDimensions[0] / this->workBlockSize[0]) * (pRenderPropertiesOpenCL->imageDimensions[1] / this->workBlockSize[1]);
  }
  if (pRenderPropertiesOpenCL->threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
    ThreadOrganizationOpenCL threadOrganization = pRenderPropertiesOpenCL->threadOrganization;
    if (threadOrganization.sType != STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL) {
      printf("ERROR: ThreadOrganizationOpenCL sType\n");
    }

    this->workBlockSize[0] = threadOrganization.workBlockSize[0];
    this->workBlockSize[1] = threadOrganization.workBlockSize[1];

    this->threadGroupSize[0] = threadOrganization.threadGroupSize[0];
    this->threadGroupSize[1] = threadOrganization.threadGroupSize[1];

    this->workBlockCount = (pRenderPropertiesOpenCL->imageDimensions[0] / this->workBlockSize[0]) * (pRenderPropertiesOpenCL->imageDimensions[1] / this->workBlockSize[1]);
  }

  cl_mem nodeBufferDevice = clCreateBuffer(this->context, CL_MEM_READ_ONLY, pAccelerationStructureExplicit->getNodeBufferSize(), NULL, NULL);
  clEnqueueWriteBuffer(this->commandQueue, nodeBufferDevice, CL_TRUE, 0, pAccelerationStructureExplicit->getNodeBufferSize(), pAccelerationStructureExplicit->getNodeBuffer(), 0, NULL, NULL);
  
  cl_mem primitiveBufferDevice = clCreateBuffer(this->context, CL_MEM_READ_ONLY, pAccelerationStructureExplicit->getOrderedPrimitiveBufferSize(), NULL, NULL);
  clEnqueueWriteBuffer(this->commandQueue, primitiveBufferDevice, CL_TRUE, 0, pAccelerationStructureExplicit->getOrderedPrimitiveBufferSize(), pAccelerationStructureExplicit->getOrderedPrimitiveBuffer(), 0, NULL, NULL);

  cl_mem materialBufferDevice = clCreateBuffer(this->context, CL_MEM_READ_ONLY, pModel->getMaterialBufferSize(), NULL, NULL);
  clEnqueueWriteBuffer(this->commandQueue, materialBufferDevice, CL_TRUE, 0, pModel->getMaterialBufferSize(), pModel->getMaterialBuffer(), 0, NULL, NULL);

  cl_mem lightContainerBufferDevice = clCreateBuffer(this->context, CL_MEM_READ_ONLY, pAccelerationStructureExplicit->getLightContainerBufferSize(), NULL, NULL);
  clEnqueueWriteBuffer(this->commandQueue, lightContainerBufferDevice, CL_TRUE, 0, pAccelerationStructureExplicit->getLightContainerBufferSize(), pAccelerationStructureExplicit->getLightContainerBuffer(), 0, NULL, NULL);

  cl_mem cameraBufferDevice = clCreateBuffer(this->context, CL_MEM_READ_ONLY, pCamera->getCameraBufferSize(), NULL, NULL);
  clEnqueueWriteBuffer(this->commandQueue, cameraBufferDevice, CL_TRUE, 0, pCamera->getCameraBufferSize(), pCamera->getCameraBuffer(), 0, NULL, NULL);

  cl_mem outputDevice = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, pRenderPropertiesOpenCL->outputBufferSize, NULL, NULL);

  cl_uint width = pRenderPropertiesOpenCL->imageDimensions[0];
  cl_uint height = pRenderPropertiesOpenCL->imageDimensions[1];
  cl_uint depth = pRenderPropertiesOpenCL->imageDimensions[2];

  cl_event events[this->workBlockCount];
  for (cl_uint x = 0; x < this->workBlockCount; x++) {
    clSetKernelArg(this->kernel, 0, sizeof(cl_mem), &nodeBufferDevice);
    clSetKernelArg(this->kernel, 1, sizeof(cl_mem), &primitiveBufferDevice);
    clSetKernelArg(this->kernel, 2, sizeof(cl_mem), &materialBufferDevice);
    clSetKernelArg(this->kernel, 3, sizeof(cl_mem), &lightContainerBufferDevice);
    clSetKernelArg(this->kernel, 4, sizeof(cl_mem), &cameraBufferDevice);
    clSetKernelArg(this->kernel, 5, sizeof(cl_mem), &outputDevice);
    clSetKernelArg(this->kernel, 6, sizeof(cl_uint), &x);
    clSetKernelArg(this->kernel, 7, sizeof(cl_uint), &width);
    clSetKernelArg(this->kernel, 8, sizeof(cl_uint), &height);
    clSetKernelArg(this->kernel, 9, sizeof(cl_uint), &depth);
    clEnqueueNDRangeKernel(this->commandQueue, this->kernel, 2, NULL, this->workBlockSize, this->threadGroupSize, 0, NULL, &events[x]);
  }
  clWaitForEvents(this->workBlockCount, events);

  clEnqueueReadBuffer(this->commandQueue, outputDevice, CL_TRUE, 0, pRenderPropertiesOpenCL->outputBufferSize, pRenderPropertiesOpenCL->pOutputBuffer, 0, NULL, NULL);
  clFinish(this->commandQueue);

  clReleaseMemObject(outputDevice);
  clReleaseKernel(this->kernel);
}