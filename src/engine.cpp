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
    stbi_write_jpg(bufferToImageProperties.filename, bufferToImageProperties.imageDimensions[0], bufferToImageProperties.imageDimensions[1], bufferToImageProperties.imageDimensions[2], bufferToImageProperties.pBuffer, 100);
  }
}