#include "renderer_cuda.h"

extern "C" {
  void linearKernel();
  void tileKernel();
}

RendererCUDA::RendererCUDA() {

}

RendererCUDA::~RendererCUDA() {

}

void RendererCUDA::render(void* pRenderProperties) {
  linearKernel();
}