#include "renderer_cuda.h"

extern "C" {
  void linearKernelWrapper(void* linearNodeBuffer,
                           uint64_t linearNodeBufferSize,
                           void* primitiveBuffer,
                           uint64_t primitiveBufferSize,
                           void* materialBuffer,
                           uint64_t materialBufferSize,
                           void* cameraBuffer,
                           uint64_t cameraBufferSize,
                           void* outputBuffer, 
                           uint64_t imageDimensions[3],
                           uint64_t blockSize[2]);

  void tileKernelWrapper();
}

RendererCUDA::RendererCUDA() {

}

RendererCUDA::~RendererCUDA() {

}

void RendererCUDA::render(void* pRenderProperties) {
  RenderPropertiesCUDA* pRenderPropertiesCUDA = (RenderPropertiesCUDA*)pRenderProperties;
  AccelerationStructure* pAccelerationStructure = (AccelerationStructure*)pRenderPropertiesCUDA->pAccelerationStructure;
  Model* pModel = (Model*)pRenderPropertiesCUDA->pModel;
  Camera* pCamera = (Camera*)pRenderPropertiesCUDA->pCamera;

  uint64_t blockSize[2];

  if (pRenderPropertiesCUDA->threadOrganizationMode == THREAD_ORGANIZATION_MODE_MAX_FIT) {
    blockSize[0] = 32;
    blockSize[1] = 32;
  }
  if (pRenderPropertiesCUDA->threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
    ThreadOrganizationCUDA* pThreadOrganization = pRenderPropertiesCUDA->pThreadOrganization;
    blockSize[0] = pThreadOrganization->blockSize[0];
    blockSize[1] = pThreadOrganization->blockSize[1];
  }

  if (pRenderPropertiesCUDA->kernelMode == KERNEL_MODE_LINEAR) {
    linearKernelWrapper(
      pAccelerationStructure->getNodeBuffer(),
      pAccelerationStructure->getNodeBufferSize(),
      pAccelerationStructure->getOrderedPrimitiveBuffer(),
      pAccelerationStructure->getOrderedPrimitiveBufferSize(),
      pModel->getMaterialBuffer(),
      pModel->getMaterialBufferSize(),
      pCamera->getCameraBuffer(),
      pCamera->getCameraBufferSize(),
      pRenderPropertiesCUDA->pOutputBuffer,
      pRenderPropertiesCUDA->imageDimensions,
      blockSize
    );
  }
  if (pRenderPropertiesCUDA->kernelMode == KERNEL_MODE_TILE) {

  }
}