#include "engine.h"
#include "model.h"
#include "acceleration_structure.h"
#include "camera.h"
#include "structures.h"

int main(int argc, const char** argv) {
  Camera* pCamera = new Camera(0, 2.5, -50, 0);
  Model* pModel = new Model("cornell_box.obj");

  AccelerationStructureProperties accelerationStructureProperties = {
    .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
    .pNext = NULL,
    .accelerationStructureType = ACCELERATION_STRUCTURE_TYPE_BVH,
    .pModel = pModel,
  };

  AccelerationStructure* pAccelerationStructure = new AccelerationStructure(accelerationStructureProperties);

  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* pOutputBuffer = malloc(outputBufferSize);

  // Engine* pEngine = new Engine(RENDER_PLATFORM_CUDA);

  // RenderPropertiesCUDA renderProperties = {
  //   .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
  //   .pNext = NULL,
  //   .kernelMode = KERNEL_MODE_TILE,
  //   .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
  //   .pThreadOrganization = NULL,
  //   .imageDimensions = {2048, 2048, 3},
  //   .pOutputBuffer = pOutputBuffer,
  //   .outputBufferSize = outputBufferSize,
  //   .pAccelerationStructure = pAccelerationStructure,
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
    .pAccelerationStructure = pAccelerationStructure,
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
    .filename = "dump/test.jpg"
  };

  pEngine->writeBufferToImage(bufferToImageProperties);

  delete pCamera;
  delete pModel;
  delete pAccelerationStructure;
  delete pEngine;
  free(pOutputBuffer);

  return 0;
}