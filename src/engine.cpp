#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "engine.h"

Engine::Engine(RenderPlatform renderPlatform) {
  this->renderPlatform = renderPlatform;

  if (renderPlatform == RENDER_PLATFORM_OPENCL) {
    this->renderer = new RendererOpenCL();
  }
}

Engine::~Engine() {
  delete this->renderer;
}

void Engine::render(void* pRenderProperties) {
  this->renderer->render(pRenderProperties);
}

void Engine::writeBufferToImage(BufferToImageProperties bufferToImageProperties) {
  if (bufferToImageProperties.sType == STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES) {
    float* imageBuffer = (float*)bufferToImageProperties.pBuffer;
    char* writeBuffer = (char*)malloc(bufferToImageProperties.bufferSize);

    uint64_t imageWidth = bufferToImageProperties.imageDimensions[0];
    uint64_t imageHeight = bufferToImageProperties.imageDimensions[1];
    uint64_t imageDepth = bufferToImageProperties.imageDimensions[2];

    for (int x = 0; x < imageWidth * imageHeight * imageDepth; x++) {
      writeBuffer[x] = imageBuffer[x];
    }

    stbi_write_jpg(bufferToImageProperties.filename, imageWidth, imageHeight, imageDepth, writeBuffer, 100);
  }
}