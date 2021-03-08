#include "renderer_cuda.h"

extern "C" {
  void linearKernelWrapper(float* pOutputBuffer, int width, int height, int depth);
  void tileKernelWrapper();
}

RendererCUDA::RendererCUDA() {

}

RendererCUDA::~RendererCUDA() {

}

void RendererCUDA::render(void* pRenderProperties) {
  RenderPropertiesCUDA* pRenderPropertiesCUDA = (RenderPropertiesCUDA*)pRenderProperties;
  linearKernelWrapper((float*)pRenderPropertiesCUDA->pOutputBuffer, pRenderPropertiesCUDA->imageDimensions[0], pRenderPropertiesCUDA->imageDimensions[1], pRenderPropertiesCUDA->imageDimensions[2]);

  for (int x = 0; x < 10; x++) {
    printf("%f\n", ((float*)(pRenderPropertiesCUDA->pOutputBuffer))[x]);
  }
}