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

void Engine::setResolution(uint64_t width, uint64_t height, uint64_t depth) {
  this->renderer->setResolution(width, height, depth);
}

void Engine::render(void* pNext) {
  this->renderer->render(pNext);
}