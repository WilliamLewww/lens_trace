
#include "engine.h"
#include "structures.h"

int main(int argc, const char** argv) {
  Engine* engine = new Engine(RENDER_PLATFORM_OPENCL);

  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* outputBuffer = malloc(outputBufferSize);

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
    .pThreadOrganization = &threadOrganization,
    .pOutputBuffer = outputBuffer,
    .outputBufferSize = outputBufferSize
  };

  engine->render(&renderProperties);

  BufferToImageProperties bufferToImageProperties = {
    .sType = STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES,
    .pNext = NULL,
    .pBuffer = outputBuffer,
    .bufferSize = outputBufferSize,
    .imageDimensions = {2048, 2048, 3},
    .imageType = IMAGE_TYPE_JPEG,
    .filename = "dump/test.jpg"
  };

  engine->writeBufferToImage(bufferToImageProperties);

  delete engine;
  free(outputBuffer);

  return 0;
}