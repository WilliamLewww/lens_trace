#include "renderer_opencl.h"

void printKernelBuildLog(cl_device_id deviceID, cl_program program) {
  char* pPrintBuffer = (char*)malloc(4096);
  uint64_t length;
  clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, 4096, pPrintBuffer, &length);
  printf("%s\n",pPrintBuffer);
  free(pPrintBuffer);
}

void printDeviceName(cl_device_id deviceID) {
  char* pDeviceNameBuffer = (char*)malloc(255);
  clGetDeviceInfo(deviceID, CL_DEVICE_NAME, 255, pDeviceNameBuffer, NULL);
  printf("%s\n", pDeviceNameBuffer);
  free(pDeviceNameBuffer);
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

  FILE* pKernelFile = fopen("src/kernels/basic.kernel", "rb");
  fseek(pKernelFile, 0, SEEK_END);
  uint32_t kernelFileSize = ftell(pKernelFile);
  fseek(pKernelFile, 0, SEEK_SET);

  char* pKernelFileBuffer = (char*)malloc(kernelFileSize + 1);
  fread(pKernelFileBuffer, 1, kernelFileSize, pKernelFile);
  fclose(pKernelFile);
  pKernelFileBuffer[kernelFileSize] = '\0';

  this->program = clCreateProgramWithSource(this->context, 1, (const char**)&pKernelFileBuffer, NULL, NULL);
  free(pKernelFileBuffer);
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
  RenderPropertiesOpenCL* pRenderPropertiesOpenCL = (RenderPropertiesOpenCL*)pRenderProperties;
  AccelerationStructure* pAccelerationStructure = (AccelerationStructure*)pRenderPropertiesOpenCL->pAccelerationStructure;

  if (pRenderPropertiesOpenCL->sType == STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL) {
    if (pRenderPropertiesOpenCL->kernelMode == KERNEL_MODE_LINEAR) {
      this->kernel = clCreateKernel(this->program, "linearKernel", NULL);
    }
    if (pRenderPropertiesOpenCL->kernelMode == KERNEL_MODE_TILE) {
      this->kernel = clCreateKernel(this->program, "tileKernel", NULL);
    }
    if (pRenderPropertiesOpenCL->threadOrganizationMode == THREAD_ORGANIZATION_MODE_MAX_FIT) {
      this->workBlockSize[0] = 64 * (this->maxWorkItemSizes[0] / 64);
      this->workBlockSize[1] = 64 * (this->maxWorkItemSizes[1] / 64);

      this->threadGroupSize[0] = 32;
      this->threadGroupSize[1] = (this->maxWorkGroupSize / 32);

      this->workBlockCount = (pRenderPropertiesOpenCL->imageDimensions[0] / this->workBlockSize[0]) * (pRenderPropertiesOpenCL->imageDimensions[1] / this->workBlockSize[1]);
    }
    if (pRenderPropertiesOpenCL->threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
      ThreadOrganizationOpenCL* pThreadOrganization = pRenderPropertiesOpenCL->pThreadOrganization;
      if (pThreadOrganization->sType == STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL) {
        this->workBlockSize[0] = pThreadOrganization->workBlockSize[0];
        this->workBlockSize[1] = pThreadOrganization->workBlockSize[1];

        this->threadGroupSize[0] = pThreadOrganization->threadGroupSize[0];
        this->threadGroupSize[1] = pThreadOrganization->threadGroupSize[1];

        this->workBlockCount = (pRenderPropertiesOpenCL->imageDimensions[0] / this->workBlockSize[0]) * (pRenderPropertiesOpenCL->imageDimensions[1] / this->workBlockSize[1]);
      }
    }
  }

  printf("Image Size: %lux%lux%lu\n", pRenderPropertiesOpenCL->imageDimensions[0], pRenderPropertiesOpenCL->imageDimensions[1], pRenderPropertiesOpenCL->imageDimensions[2]);
  printf("Work Block Size: %lux%lu\n", this->workBlockSize[0], this->workBlockSize[1]);
  printf("Thread Group Size: %lux%lu\n", this->threadGroupSize[0], this->threadGroupSize[1]);
  printf("Work Block Count: %lu\n", this->workBlockCount);

  cl_mem nodeBufferDevice = clCreateBuffer(this->context, CL_MEM_READ_ONLY, pAccelerationStructure->getNodeBufferSize(), NULL, NULL);
  clEnqueueWriteBuffer(this->commandQueue, nodeBufferDevice, CL_TRUE, 0, pAccelerationStructure->getNodeBufferSize(), pAccelerationStructure->getNodeBuffer(), 0, NULL, NULL);
  
  cl_mem primitiveBufferDevice = clCreateBuffer(this->context, CL_MEM_READ_ONLY, pAccelerationStructure->getOrderedVertexBufferSize(), NULL, NULL);
  clEnqueueWriteBuffer(this->commandQueue, primitiveBufferDevice, CL_TRUE, 0, pAccelerationStructure->getOrderedVertexBufferSize(), pAccelerationStructure->getOrderedVertexBuffer(), 0, NULL, NULL);

  cl_mem outputDevice = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, pRenderPropertiesOpenCL->outputBufferSize, NULL, NULL);

  cl_uint width = pRenderPropertiesOpenCL->imageDimensions[0];
  cl_uint height = pRenderPropertiesOpenCL->imageDimensions[1];
  cl_uint depth = pRenderPropertiesOpenCL->imageDimensions[2];

  clock_t start = clock();
  clock_t end;

  cl_event events[this->workBlockCount];
  for (cl_uint x = 0; x < this->workBlockCount; x++) {
    clSetKernelArg(this->kernel, 0, sizeof(cl_mem), &nodeBufferDevice);
    clSetKernelArg(this->kernel, 1, sizeof(cl_mem), &primitiveBufferDevice);
    clSetKernelArg(this->kernel, 2, sizeof(cl_mem), &outputDevice);
    clSetKernelArg(this->kernel, 3, sizeof(cl_uint), &x);
    clSetKernelArg(this->kernel, 4, sizeof(cl_uint), &width);
    clSetKernelArg(this->kernel, 5, sizeof(cl_uint), &height);
    clSetKernelArg(this->kernel, 6, sizeof(cl_uint), &depth);
    clEnqueueNDRangeKernel(this->commandQueue, this->kernel, 2, NULL, this->workBlockSize, this->threadGroupSize, 0, NULL, &events[x]);
  }
  clWaitForEvents(this->workBlockCount, events);

  end = clock();

  double timeSeconds = (double)(end - start) / (double)CLOCKS_PER_SEC;
  printf("Kernel Execution Time: %lf\n", timeSeconds);

  clEnqueueReadBuffer(this->commandQueue, outputDevice, CL_TRUE, 0, pRenderPropertiesOpenCL->outputBufferSize, pRenderPropertiesOpenCL->pOutputBuffer, 0, NULL, NULL);
  clFinish(this->commandQueue);

  clReleaseMemObject(outputDevice);
  clReleaseKernel(this->kernel);
}