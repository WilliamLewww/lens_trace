
#include "engine.h"
#include "structures.h"

int main(int argc, const char** argv) {
  Engine* engine = new Engine(RENDER_PLATFORM_OPENCL);

  ThreadOrganizationOpenCL threadOrganization = {
    .sType = STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL,
    .pNext = NULL,
    .workBlockSize = {64, 64},
    .threadGroupSize = {32, 32}
  };

  RenderPropertiesOpenCL renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
    .pNext = NULL,
    .kernelMode = KERNEL_MODE_LINEAR,
    .imageDimensions = {2048, 2048, 3},
    .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
    .pThreadOrganization = &threadOrganization
  };

  engine->render(&renderProperties);

  delete engine;
  return 0;
}