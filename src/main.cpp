#include "engine.h"
#include "model.h"
#include "acceleration_structure_optix.h"
#include "camera.h"
#include "structures.h"

int main(int argc, const char** argv) {
  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* pOutputBuffer = malloc(outputBufferSize);

  Camera* pCamera = new Camera(0, 2.5, -50, 0);
  Model* pModel = new Model("cornell_box.obj");

  AccelerationStructureExplicitProperties accelerationStructureExplicitProperties = {
    .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
    .pNext = NULL,
    .accelerationStructureExplicitType = ACCELERATION_STRUCTURE_TYPE_BVH,
    .pModel = pModel,
  };
  AccelerationStructureExplicit* pAccelerationStructureExplicit = new AccelerationStructureExplicit(accelerationStructureExplicitProperties);

  // Engine* pEngine = new Engine(RENDER_PLATFORM_CUDA);
  // RenderPropertiesCUDA renderProperties = {
  //   .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
  //   .pNext = NULL,
  //   .kernelMode = KERNEL_MODE_LINEAR,
  //   .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
  //   .pThreadOrganization = NULL,
  //   .imageDimensions = {2048, 2048, 3},
  //   .pOutputBuffer = pOutputBuffer,
  //   .outputBufferSize = outputBufferSize,
  //   .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
  //   .pModel = pModel,
  //   .pCamera = pCamera
  // };

  Engine* pEngine = new Engine(RENDER_PLATFORM_OPENCL);
  RenderPropertiesOpenCL renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
    .pNext = NULL,
    .kernelMode = KERNEL_MODE_LINEAR,
    .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
    .pThreadOrganization = NULL,
    .imageDimensions = {2048, 2048, 3},
    .pOutputBuffer = pOutputBuffer,
    .outputBufferSize = outputBufferSize,
    .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
    .pModel = pModel,
    .pCamera = pCamera
  };

  pEngine->render(&renderProperties);

  BufferToImageProperties bufferToImageProperties = {
    .sType = STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES,
    .pNext = NULL,
    .pBuffer = pOutputBuffer,
    .bufferSize = outputBufferSize,
    .imageDimensions = {2048, 2048, 3},
    .imageType = IMAGE_TYPE_JPEG,
    .filename = "test.jpg"
  };
  pEngine->writeBufferToImage(bufferToImageProperties);

  delete pCamera;
  delete pModel;
  delete pAccelerationStructureExplicit;
  delete pEngine;
  free(pOutputBuffer);

  return 0;
}