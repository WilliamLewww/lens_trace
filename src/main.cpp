
#include "engine.h"

int main(int argc, const char** argv) {
  Engine* engine = new Engine(RENDER_PLATFORM_OPENCL);
  engine->setResolution(2048, 2048, 1);

  RenderPropertiesOpenCL renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
    .pNext = NULL,
    .kernelMode = KERNEL_MODE_LINEAR
  };

  engine->render(&renderProperties);

  delete engine;
  return 0;
}