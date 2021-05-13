#include "lens_trace/cuda/renderer_cuda.h"

extern "C" {
  void basic_cuda_kernelWrappers(void* linearNodeBuffer,
                                 uint64_t linearNodeBufferSize,
                                 void* primitiveBuffer,
                                 uint64_t primitiveBufferSize,
                                 void* materialBuffer,
                                 uint64_t materialBufferSize,
                                 void* cameraBuffer,
                                 uint64_t cameraBufferSize,
                                 void* outputBuffer, 
                                 uint64_t imageDimensions[3],
                                 uint64_t blockSize[2],
                                 KernelMode kernelMode);
}

std::map<std::string, void (*)(KERNEL_ARGUMENTS)> RendererCUDA::kernelMap = {
  {"basic_cuda", basic_cuda_kernelWrappers},
};

RendererCUDA::RendererCUDA() {

}

RendererCUDA::~RendererCUDA() {

}

void RendererCUDA::render(void* pRenderProperties) {
  RenderPropertiesCUDA* pRenderPropertiesCUDA = (RenderPropertiesCUDA*)pRenderProperties;

  if (pRenderPropertiesCUDA->sType != STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA) {
    printf("ERROR: RenderPropertiesCUDA sType\n");
  }

  AccelerationStructureExplicit* pAccelerationStructureExplicit = (AccelerationStructureExplicit*)pRenderPropertiesCUDA->pAccelerationStructureExplicit;
  Model* pModel = (Model*)pRenderPropertiesCUDA->pModel;
  Camera* pCamera = (Camera*)pRenderPropertiesCUDA->pCamera;

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

  kernelMap[pRenderPropertiesCUDA->kernelName](
    pAccelerationStructureExplicit->getNodeBuffer(),
    pAccelerationStructureExplicit->getNodeBufferSize(),
    pAccelerationStructureExplicit->getOrderedPrimitiveBuffer(),
    pAccelerationStructureExplicit->getOrderedPrimitiveBufferSize(),
    pModel->getMaterialBuffer(),
    pModel->getMaterialBufferSize(),
    pCamera->getCameraBuffer(),
    pCamera->getCameraBufferSize(),
    pRenderPropertiesCUDA->pOutputBuffer,
    pRenderPropertiesCUDA->imageDimensions,
    blockSize,
    pRenderPropertiesCUDA->kernelMode
  );
}