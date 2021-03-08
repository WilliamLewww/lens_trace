#include "renderer_cuda.h"

extern "C" {
  void linearKernelWrapper(float* pOutputBuffer, int width, int height);
  void tileKernelWrapper();
}

RendererCUDA::RendererCUDA() {

}

RendererCUDA::~RendererCUDA() {

}

void RendererCUDA::render(void* pRenderProperties) {
  float* pOutputBuffer = (float*)malloc(sizeof(float) * 2048 * 2048);
  linearKernelWrapper(pOutputBuffer, 2048, 2048);

  for (int x = 0; x < 10; x++) {
    printf("%f\n", pOutputBuffer[x]);
  }

  free(pOutputBuffer);
}