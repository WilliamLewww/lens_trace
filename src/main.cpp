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

  Engine* pEngine = new Engine(RENDER_PLATFORM_CUDA);

  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* pOutputBuffer = malloc(outputBufferSize);

  RenderPropertiesCUDA renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
    .pNext = NULL,
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