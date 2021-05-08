#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#include "lens_trace/engine.h"

Engine::Engine(RenderPlatform renderPlatform) {
  this->renderPlatform = renderPlatform;

#ifdef OPENCL_ENABLED
  if (renderPlatform == RENDER_PLATFORM_OPENCL) {
    this->pRenderer = new RendererOpenCL();
  }
#endif

#ifdef CUDA_ENABLED
  if (renderPlatform == RENDER_PLATFORM_CUDA) {
    this->pRenderer = new RendererCUDA();
  }
#endif

#ifdef OPTIX_ENABLED
  if (renderPlatform == RENDER_PLATFORM_OPTIX) {
    this->pRenderer = new RendererOptiX();
  }
#endif
}

Engine::~Engine() {
  delete this->pRenderer;
}

void Engine::render(void* pRenderProperties) {
  this->pRenderer->render(pRenderProperties);
}

void Engine::writeBufferToImage(BufferToImageProperties bufferToImageProperties) {
  if (bufferToImageProperties.sType == STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES) {
    float* pImageBuffer = (float*)bufferToImageProperties.pBuffer;
    char* pWriteBuffer = (char*)malloc(bufferToImageProperties.bufferSize);

    uint64_t imageWidth = bufferToImageProperties.imageDimensions[0];
    uint64_t imageHeight = bufferToImageProperties.imageDimensions[1];
    uint64_t imageDepth = bufferToImageProperties.imageDimensions[2];

    for (int x = 0; x < imageWidth * imageHeight * imageDepth; x++) {
      // pWriteBuffer[(imageWidth * imageHeight * (x % 3)) + (x / 3)] = pImageBuffer[x];
      pWriteBuffer[x] = pImageBuffer[x] * 255;
    }

    stbi_write_jpg(bufferToImageProperties.filename, imageWidth, imageHeight, imageDepth, pWriteBuffer, 100);
    free(pWriteBuffer);
  }
}