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

void Engine::render(void* pNext) {
  this->renderer->render(pNext);
}