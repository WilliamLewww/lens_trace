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
                           int width, 
                           int height, 
                           int depth);

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
    pRenderPropertiesCUDA->imageDimensions[0], 
    pRenderPropertiesCUDA->imageDimensions[1], 
    pRenderPropertiesCUDA->imageDimensions[2]
  );
}