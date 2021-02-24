
#include "engine.h"

int main(int argc, const char** argv) {
  Engine* engine = new Engine(RENDER_PLATFORM_OPENCL);

  RenderPropertiesOpenCL renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
    .pNext = NULL,
    .kernelMode = KERNEL_MODE_LINEAR,
    .imageWidth = 2048,
    .imageHeight = 2048,
    .imageDepth = 1
  };

  engine->render(&renderProperties);

  delete engine;
  return 0;
}