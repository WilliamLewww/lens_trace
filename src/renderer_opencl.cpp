#include "renderer_opencl.h"

void printKernelBuildLog(cl_device_id deviceID, cl_program program) {
  char buffer[4096];
  uint64_t length;
  clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
  printf("%s\n",buffer);
}

void printDeviceName(cl_device_id deviceID) {
  char* deviceName = (char*)malloc(255);
  clGetDeviceInfo(deviceID, CL_DEVICE_NAME, 255, deviceName, NULL);
  printf("%s\n", deviceName);
  free(deviceName);
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

  FILE* kernelFile = fopen("src/kernels/basic.kernel", "rb");
  fseek(kernelFile, 0, SEEK_END);
  uint32_t kernelFileSize = ftell(kernelFile);
  fseek(kernelFile, 0, SEEK_SET);

  char* kernelFileBuffer = (char*)malloc(kernelFileSize + 1);
  fread(kernelFileBuffer, 1, kernelFileSize, kernelFile);
  fclose(kernelFile);
  kernelFileBuffer[kernelFileSize] = '\0';

  this->program = clCreateProgramWithSource(this->context, 1, (const char**)&kernelFileBuffer, NULL, NULL);
  free(kernelFileBuffer);
  clBuildProgram(this->program, 0, NULL, NULL, NULL, NULL);

  printDeviceName(this->deviceID);
  printKernelBuildLog(this->deviceID, this->program);
}

RendererOpenCL::~RendererOpenCL() {
  clReleaseProgram(this->program);
  clReleaseCommandQueue(this->commandQueue);
  clReleaseContext(this->context);
}

void RendererOpenCL::render(void* pRenderProperties) {
  RenderPropertiesOpenCL* renderPropertiesOpenCL = (RenderPropertiesOpenCL*)pRenderProperties;
  AccelerationStructure* accelerationStructure = (AccelerationStructure*)renderPropertiesOpenCL->pAccelerationStructure;

  if (renderPropertiesOpenCL->sType == STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL) {
    if (renderPropertiesOpenCL->kernelMode == KERNEL_MODE_LINEAR) {
      this->kernel = clCreateKernel(this->program, "linearKernel", NULL);
    }
    if (renderPropertiesOpenCL->kernelMode == KERNEL_MODE_TILE) {
      this->kernel = clCreateKernel(this->program, "tileKernel", NULL);
    }
    if (renderPropertiesOpenCL->threadOrganizationMode == THREAD_ORGANIZATION_MODE_MAX_FIT) {
      this->workBlockSize[0] = 64 * (this->maxWorkItemSizes[0] / 64);
      this->workBlockSize[1] = 64 * (this->maxWorkItemSizes[1] / 64);

      this->threadGroupSize[0] = 32;
      this->threadGroupSize[1] = (this->maxWorkGroupSize / 32);

      this->workBlockCount = (renderPropertiesOpenCL->imageDimensions[0] / this->workBlockSize[0]) * (renderPropertiesOpenCL->imageDimensions[1] / this->workBlockSize[1]);
    }
    if (renderPropertiesOpenCL->threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
      ThreadOrganizationOpenCL* threadOrganization = renderPropertiesOpenCL->pThreadOrganization;
      if (threadOrganization->sType == STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL) {
        this->workBlockSize[0] = threadOrganization->workBlockSize[0];
        this->workBlockSize[1] = threadOrganization->workBlockSize[1];

        this->threadGroupSize[0] = threadOrganization->threadGroupSize[0];
        this->threadGroupSize[1] = threadOrganization->threadGroupSize[1];

        this->workBlockCount = (renderPropertiesOpenCL->imageDimensions[0] / this->workBlockSize[0]) * (renderPropertiesOpenCL->imageDimensions[1] / this->workBlockSize[1]);
      }
    }
  }

  printf("Image Size: %lux%lux%lu\n", renderPropertiesOpenCL->imageDimensions[0], renderPropertiesOpenCL->imageDimensions[1], renderPropertiesOpenCL->imageDimensions[2]);
  printf("Work Block Size: %lux%lu\n", this->workBlockSize[0], this->workBlockSize[1]);
  printf("Thread Group Size: %lux%lu\n", this->threadGroupSize[0], this->threadGroupSize[1]);
  printf("Work Block Count: %lu\n", this->workBlockCount);

  cl_mem nodeBufferDevice = clCreateBuffer(this->context, CL_MEM_READ_ONLY, accelerationStructure->getNodeBufferSize(), NULL, NULL);
  clEnqueueWriteBuffer(this->commandQueue, nodeBufferDevice, CL_TRUE, 0, accelerationStructure->getNodeBufferSize(), accelerationStructure->getNodeBuffer(), 0, NULL, NULL);
  
  cl_mem outputDevice = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, renderPropertiesOpenCL->outputBufferSize, NULL, NULL);

  cl_uint width = renderPropertiesOpenCL->imageDimensions[0];
  cl_uint height = renderPropertiesOpenCL->imageDimensions[1];
  cl_uint depth = renderPropertiesOpenCL->imageDimensions[2];

  cl_event events[this->workBlockCount];
  for (cl_uint x = 0; x < this->workBlockCount; x++) {
    clSetKernelArg(this->kernel, 0, sizeof(cl_mem), &nodeBufferDevice);
    clSetKernelArg(this->kernel, 1, sizeof(cl_mem), &outputDevice);
    clSetKernelArg(this->kernel, 2, sizeof(cl_uint), &x);
    clSetKernelArg(this->kernel, 3, sizeof(cl_uint), &width);
    clSetKernelArg(this->kernel, 4, sizeof(cl_uint), &height);
    clSetKernelArg(this->kernel, 5, sizeof(cl_uint), &depth);
    clEnqueueNDRangeKernel(this->commandQueue, this->kernel, 2, NULL, this->workBlockSize, this->threadGroupSize, 0, NULL, &events[x]);
  }
  clWaitForEvents(this->workBlockCount, events);

  clEnqueueReadBuffer(this->commandQueue, outputDevice, CL_TRUE, 0, renderPropertiesOpenCL->outputBufferSize, renderPropertiesOpenCL->pOutputBuffer, 0, NULL, NULL);
  clFinish(this->commandQueue);

  clReleaseMemObject(outputDevice);
  clReleaseKernel(this->kernel);
}