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

  printDeviceName(this->deviceID);

  clGetDeviceInfo(this->deviceID, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(this->maxWorkItemSizes), &this->maxWorkItemSizes, NULL);
  clGetDeviceInfo(this->deviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(this->maxWorkGroupSize), &this->maxWorkGroupSize, NULL);

  this->contextProperties[0] = CL_CONTEXT_PLATFORM;
  this->contextProperties[1] = (cl_context_properties)this->platformID;
  this->contextProperties[2] = 0;

  cl_int error;
  this->context = clCreateContext(this->contextProperties, 1, &this->deviceID, NULL, NULL, &error);
  this->commandQueue = clCreateCommandQueueWithProperties(this->context, this->deviceID, NULL, &error);

  FILE* kernelFile = fopen("src/kernels/basic.kernel", "rb");
  fseek(kernelFile, 0, SEEK_END);
  uint32_t kernelFileSize = ftell(kernelFile);
  fseek(kernelFile, 0, SEEK_SET);

  char* kernelFileBuffer = (char*)malloc(kernelFileSize + 1);
  fread(kernelFileBuffer, 1, kernelFileSize, kernelFile);
  fclose(kernelFile);
  kernelFileBuffer[kernelFileSize] = '\0';

  this->program = clCreateProgramWithSource(this->context, 1, (const char**)&kernelFileBuffer, NULL, &error);
  clBuildProgram(this->program, 0, NULL, NULL, NULL, NULL);
  printKernelBuildLog(this->deviceID, this->program);
  free(kernelFileBuffer);
}

RendererOpenCL::~RendererOpenCL() {
  clReleaseProgram(this->program);
  clReleaseCommandQueue(this->commandQueue);
  clReleaseContext(this->context);
}

void RendererOpenCL::setResolution(uint64_t width, uint64_t height, uint64_t depth) {
  this->imageWidth = width;
  this->imageHeight = height;
  this->imageDepth = depth;
}

void RendererOpenCL::render(void* pNext) {
  cl_int error;
  cl_kernel kernel;

  if (pNext) {
    RenderPropertiesOpenCL* renderProperties = (RenderPropertiesOpenCL*)pNext;
    if (renderProperties->sType == STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL) {
      if (renderProperties->kernelMode == KERNEL_MODE_LINEAR) {
        kernel = clCreateKernel(this->program, "linearKernel", &error);
      }
      if (renderProperties->kernelMode == KERNEL_MODE_TILE) {
        kernel = clCreateKernel(this->program, "tileKernel", &error);
      }
    }
  }
  else {
    kernel = clCreateKernel(this->program, "linearKernel", &error);
  }

  uint64_t workBlockSize[2] = {64 * (this->maxWorkItemSizes[0] / 64), 64 * (this->maxWorkItemSizes[1] / 64)};
  uint64_t threadGroupSize[2] = {32, (this->maxWorkGroupSize / 32)};
  uint64_t workBlockCount = (this->imageWidth / workBlockSize[0]) * (this->imageHeight / workBlockSize[1]);

  printf("Image Size: %lux%lu\n", this->imageWidth, this->imageHeight);
  printf("Work Block Size: %lux%lu\n", workBlockSize[0], workBlockSize[1]);
  printf("Thread Group Size: %lux%lu\n", threadGroupSize[0], threadGroupSize[1]);
  printf("Work Block Count: %lu\n", workBlockCount);

  float* outputHost = (float*)malloc(sizeof(float) * this->imageWidth * this->imageHeight);
  cl_mem outputDevice = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(float) * this->imageWidth * this->imageHeight, NULL, NULL);

  cl_uint width = this->imageWidth;
  cl_uint height = this->imageHeight;

  cl_event events[workBlockCount];
  for (cl_uint x = 0; x < workBlockCount; x++) {
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &outputDevice);
    clSetKernelArg(kernel, 1, sizeof(cl_uint), &x);
    clSetKernelArg(kernel, 2, sizeof(cl_uint), &width);
    clSetKernelArg(kernel, 3, sizeof(cl_uint), &height);
    clEnqueueNDRangeKernel(this->commandQueue, kernel, 2, NULL, workBlockSize, threadGroupSize, 0, NULL, &events[x]);
  }
  clWaitForEvents(workBlockCount, events);

  clEnqueueReadBuffer(this->commandQueue, outputDevice, CL_TRUE, 0, sizeof(float) * this->imageWidth * this->imageHeight, outputHost, 0, NULL, NULL);
  clFinish(this->commandQueue);

  for (int x = 0; x < workBlockCount; x++) {
    printf("Block #%d: %f\n", x, outputHost[x * workBlockSize[0] * workBlockSize[1]]);
  }

  clReleaseMemObject(outputDevice);
  free(outputHost);
  
  clReleaseKernel(kernel);
}