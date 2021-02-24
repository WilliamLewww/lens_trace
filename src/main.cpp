#define CL_TARGET_OPENCL_VERSION 300

#include <stdio.h>
#include <time.h>

#include <CL/cl.h>

void printKernelBuildLog(cl_device_id deviceID, cl_program program) {
  char buffer[4096];
  uint64_t length;
  clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
  printf("%s\n",buffer);
}

int main(int argc, const char** argv) {
  cl_uint platformCount = 0;
  cl_platform_id platformID;
  clGetPlatformIDs(1, &platformID, &platformCount);

  cl_device_id deviceID;
  cl_uint deviceCount = 0;
  clGetDeviceIDs(platformID, CL_DEVICE_TYPE_GPU, 1, &deviceID, &deviceCount);

  char* deviceName = (char*)malloc(255);
  clGetDeviceInfo(deviceID, CL_DEVICE_NAME, 255, deviceName, NULL);
  printf("%s\n", deviceName);
  free(deviceName);

  uint64_t maxWorkItemSizes[3];
  uint64_t maxWorkGroupSize;
  clGetDeviceInfo(deviceID, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), &maxWorkItemSizes, NULL);
  clGetDeviceInfo(deviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, NULL);

  cl_int error;
  cl_context_properties contextProperties[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platformID, 0 };
  cl_context context = clCreateContext(contextProperties, 1, &deviceID, NULL, NULL, &error);

  cl_command_queue commandQueue = clCreateCommandQueueWithProperties(context, deviceID, NULL, &error);

  FILE* kernelFile = fopen("src/kernels/basic.kernel", "rb");
  fseek(kernelFile, 0, SEEK_END);
  uint32_t kernelFileSize = ftell(kernelFile);
  fseek(kernelFile, 0, SEEK_SET);

  char* kernelFileBuffer = (char*)malloc(kernelFileSize + 1);
  fread(kernelFileBuffer, 1, kernelFileSize, kernelFile);
  fclose(kernelFile);
  kernelFileBuffer[kernelFileSize] = '\0';

  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&kernelFileBuffer, NULL, &error);
  clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  printKernelBuildLog(deviceID, program);
  free(kernelFileBuffer);

  cl_kernel kernel = clCreateKernel(program, "squareKernel", &error);

  #define IMAGE_WIDTH 2048
  #define IMAGE_HEIGHT 2048

  printf("%lu %lu | %lu \n", maxWorkItemSizes[0], maxWorkItemSizes[1], maxWorkGroupSize);

  uint64_t workBlockSize[2] = {64 * (maxWorkItemSizes[0] / 64), 64 * (maxWorkItemSizes[1] / 64)};
  uint64_t threadGroupSize[2] = {32, (maxWorkGroupSize / 32)};

  printf("%lu %lu | %lu %lu\n", workBlockSize[0], workBlockSize[1], threadGroupSize[0], threadGroupSize[1]);

  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(commandQueue);
  clReleaseContext(context);

  return 0;
}