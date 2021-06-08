#include "lens_trace/cuda/renderer_cuda.h"

void printKernelBuildLog(nvrtcProgram program) {
  char* pPrintBuffer = (char*)malloc(4096);
  nvrtcGetProgramLog(program, pPrintBuffer);
  printf("%s\n", pPrintBuffer);
  free(pPrintBuffer);
}

RendererCUDA::RendererCUDA() {
  cuInit(0);
  cuDeviceGet(&this->device, 0);
  cuCtxCreate(&this->context, 0, this->device);
}

RendererCUDA::~RendererCUDA() {

}

void RendererCUDA::compileKernel(std::string kernelFilePath) {
  std::string basicKernelFileName = Resource::findResource(kernelFilePath.c_str());
  FILE* pKernelFile = fopen(basicKernelFileName.c_str(), "rb");
  fseek(pKernelFile, 0, SEEK_END);
  uint32_t kernelFileSize = ftell(pKernelFile);
  fseek(pKernelFile, 0, SEEK_SET);

  char* pKernelFileBuffer = (char*)malloc(kernelFileSize + 1);
  fread(pKernelFileBuffer, 1, kernelFileSize, pKernelFile);
  fclose(pKernelFile);
  pKernelFileBuffer[kernelFileSize] = '\0';

  nvrtcCreateProgram(&this->programMap[kernelFilePath], pKernelFileBuffer, kernelFilePath.c_str(), 0, NULL, NULL);
  free(pKernelFileBuffer);

  nvrtcResult error = nvrtcCompileProgram(this->programMap[kernelFilePath], 0, NULL);
  if (error != NVRTC_SUCCESS) {
    printKernelBuildLog(this->programMap[kernelFilePath]);
  }
}

void RendererCUDA::render(void* pRenderProperties) {
  RenderPropertiesCUDA* pRenderPropertiesCUDA = (RenderPropertiesCUDA*)pRenderProperties;

  if (pRenderPropertiesCUDA->sType != STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA) {
    printf("ERROR: RenderPropertiesCUDA sType\n");
  }

  AccelerationStructureExplicit* pAccelerationStructureExplicit = (AccelerationStructureExplicit*)pRenderPropertiesCUDA->pAccelerationStructureExplicit;
  Model* pModel = (Model*)pRenderPropertiesCUDA->pModel;
  Camera* pCamera = (Camera*)pRenderPropertiesCUDA->pCamera;

  if (this->programMap.find(pRenderPropertiesCUDA->kernelFilePath) == this->programMap.end()) {
    compileKernel(pRenderPropertiesCUDA->kernelFilePath);
  }
  nvrtcProgram program = this->programMap[pRenderPropertiesCUDA->kernelFilePath];

  size_t ptxSize;
  nvrtcGetPTXSize(program, &ptxSize);
  char* ptx = new char[ptxSize];
  nvrtcGetPTX(program, ptx);

  CUmodule module;
  CUfunction kernel;

  cuModuleLoadDataEx(&module, ptx, 0, 0, 0);

  if (pRenderPropertiesCUDA->kernelMode == KERNEL_MODE_LINEAR) {
    cuModuleGetFunction(&kernel, module, "linearKernel");
  }
  if (pRenderPropertiesCUDA->kernelMode == KERNEL_MODE_TILE) {
    cuModuleGetFunction(&kernel, module, "tileKernel");
  }

  uint64_t blockSize[2];
  if (pRenderPropertiesCUDA->threadOrganizationMode == THREAD_ORGANIZATION_MODE_MAX_FIT) {
    blockSize[0] = 32;
    blockSize[1] = 1;
  }
  if (pRenderPropertiesCUDA->threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
    ThreadOrganizationCUDA threadOrganization = pRenderPropertiesCUDA->threadOrganization;
    if (threadOrganization.sType != STRUCTURE_TYPE_THREAD_ORGANIZATION_CUDA) {
      printf("ERROR: ThreadOrganizationCUDA sType\n");
    }

    blockSize[0] = threadOrganization.blockSize[0];
    blockSize[1] = threadOrganization.blockSize[1];
  }
  uint64_t gridSize[2] = {(pRenderPropertiesCUDA->imageDimensions[0] + blockSize[0] - 1) / blockSize[0], (pRenderPropertiesCUDA->imageDimensions[1] + blockSize[1] - 1) / blockSize[1]};

  CUdeviceptr linearNodeBufferDevice;
  cuMemAlloc(&linearNodeBufferDevice, pAccelerationStructureExplicit->getNodeBufferSize());
  cuMemcpyHtoD(linearNodeBufferDevice, pAccelerationStructureExplicit->getNodeBuffer(), pAccelerationStructureExplicit->getNodeBufferSize());

  CUdeviceptr primitiveBufferDevice;
  cuMemAlloc(&primitiveBufferDevice, pAccelerationStructureExplicit->getOrderedPrimitiveBufferSize());
  cuMemcpyHtoD(primitiveBufferDevice, pAccelerationStructureExplicit->getOrderedPrimitiveBuffer(), pAccelerationStructureExplicit->getOrderedPrimitiveBufferSize());

  CUdeviceptr materialBufferDevice;
  cuMemAlloc(&materialBufferDevice, pModel->getMaterialBufferSize());
  cuMemcpyHtoD(materialBufferDevice, pModel->getMaterialBuffer(), pModel->getMaterialBufferSize());

  CUdeviceptr lightContainerBufferDevice;
  cuMemAlloc(&lightContainerBufferDevice, pAccelerationStructureExplicit->getLightContainerBufferSize());
  cuMemcpyHtoD(lightContainerBufferDevice, pAccelerationStructureExplicit->getLightContainerBuffer(), pAccelerationStructureExplicit->getLightContainerBufferSize());

  CUdeviceptr cameraBufferDevice;
  cuMemAlloc(&cameraBufferDevice, pCamera->getCameraBufferSize());
  cuMemcpyHtoD(cameraBufferDevice, pCamera->getCameraBuffer(), pCamera->getCameraBufferSize());

  CUdeviceptr outputBufferDevice;
  cuMemAlloc(&outputBufferDevice, sizeof(float) * pRenderPropertiesCUDA->imageDimensions[0] * pRenderPropertiesCUDA->imageDimensions[1] * pRenderPropertiesCUDA->imageDimensions[2]);

  void* args[] = { 
    &linearNodeBufferDevice, 
    &primitiveBufferDevice, 
    &materialBufferDevice, 
    &lightContainerBufferDevice, 
    &cameraBufferDevice, 
    &outputBufferDevice,
    &pRenderPropertiesCUDA->imageDimensions[0],
    &pRenderPropertiesCUDA->imageDimensions[1],
    &pRenderPropertiesCUDA->imageDimensions[2]
  };

  CUresult result = cuLaunchKernel(
    kernel,
    gridSize[0], gridSize[1], 1,
    blockSize[0], blockSize[1], 1,
    0, 
    NULL,
    args,
    0
  );
  if (result != CUDA_SUCCESS) {
    printf("Kernel Error: %d\n", result);
  }

  cuCtxSynchronize();
  cuMemcpyDtoH(pRenderPropertiesCUDA->pOutputBuffer, outputBufferDevice, sizeof(float) * pRenderPropertiesCUDA->imageDimensions[0] * pRenderPropertiesCUDA->imageDimensions[1] * pRenderPropertiesCUDA->imageDimensions[2]);
}