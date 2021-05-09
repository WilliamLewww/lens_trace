#include <gtest/gtest.h>

#include "lens_trace/cuda/renderer_cuda.h"

TEST (CreateEngineTEST, ValidEngine) {
  RendererCUDA* renderer = new RendererCUDA();
  EXPECT_TRUE(renderer != NULL);

  delete renderer;
}

TEST (RenderBufferTEST, ValidBuffer) {
  RendererCUDA* renderer = new RendererCUDA();
  EXPECT_TRUE(renderer != NULL);

  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* pOutputBuffer = malloc(outputBufferSize);

  Camera* pCamera = new Camera(0, 2.5, -50, 0);
  Model* pModel = new Model("resources/models/green_wall.obj");

  AccelerationStructureExplicitProperties accelerationStructureExplicitProperties = {
    .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
    .pNext = NULL,
    .accelerationStructureExplicitType = ACCELERATION_STRUCTURE_TYPE_BVH,
    .pModel = pModel,
  };
  AccelerationStructureExplicit* pAccelerationStructureExplicit = new AccelerationStructureExplicit(accelerationStructureExplicitProperties);

  RenderPropertiesCUDA renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
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

  renderer->render(&renderProperties);

  delete renderer;
}

TEST (RenderBufferTEST, CustomBlockSize) {
  RendererCUDA* renderer = new RendererCUDA();
  EXPECT_TRUE(renderer != NULL);

  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* pOutputBufferA = malloc(outputBufferSize);
  void* pOutputBufferB = malloc(outputBufferSize);
  void* pOutputBufferC = malloc(outputBufferSize);

  Camera* pCamera = new Camera(0, 2.5, -50, 0);
  Model* pModel = new Model("resources/models/green_wall.obj");

  AccelerationStructureExplicitProperties accelerationStructureExplicitProperties = {
    .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
    .pNext = NULL,
    .accelerationStructureExplicitType = ACCELERATION_STRUCTURE_TYPE_BVH,
    .pModel = pModel,
  };
  AccelerationStructureExplicit* pAccelerationStructureExplicit = new AccelerationStructureExplicit(accelerationStructureExplicitProperties);

  RenderPropertiesCUDA renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
    .pNext = NULL,
    .kernelMode = KERNEL_MODE_LINEAR,
    .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
    .pThreadOrganization = NULL,
    .imageDimensions = {2048, 2048, 3},
    .pOutputBuffer = pOutputBufferA,
    .outputBufferSize = outputBufferSize,
    .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
    .pModel = pModel,
    .pCamera = pCamera
  };

  renderer->render(&renderProperties);

  renderProperties.pOutputBuffer = pOutputBufferB;
  renderProperties.threadOrganizationMode = THREAD_ORGANIZATION_MODE_CUSTOM;
  ThreadOrganizationCUDA threadOrganization = {
    .sType = STRUCTURE_TYPE_THREAD_ORGANIZATION_CUDA,
    .pNext = NULL,
    .blockSize = {8, 8}
  };
  renderProperties.pThreadOrganization = &threadOrganization;

  renderer->render(&renderProperties);

  renderProperties.pOutputBuffer = pOutputBufferC;
  threadOrganization.blockSize[0] = 1;
  threadOrganization.blockSize[1] = 1;

  renderer->render(&renderProperties);

  for (int x = 0; x < 2048 * 2048; x += 32) {
    EXPECT_FLOAT_EQ(((float*)pOutputBufferA)[x], ((float*)pOutputBufferB)[x]);
    EXPECT_FLOAT_EQ(((float*)pOutputBufferB)[x], ((float*)pOutputBufferC)[x]);
  }

  delete renderer;
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}