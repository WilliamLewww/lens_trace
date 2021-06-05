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

  uint64_t outputBufferSize = sizeof(float) * 800 * 800 * 3;
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
    .kernelName = "basic_cuda",
    .kernelMode = KERNEL_MODE_LINEAR,
    .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
    .threadOrganization = {},
    .imageDimensions = {800, 800, 3},
    .pOutputBuffer = pOutputBuffer,
    .outputBufferSize = outputBufferSize,
    .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
    .pModel = pModel,
    .pCamera = pCamera
  };

  renderer->render(&renderProperties);

  delete renderer;
  free(pOutputBuffer);
}

TEST (RenderBufferTEST, CorrectColor) {
  RendererCUDA* renderer = new RendererCUDA();
  EXPECT_TRUE(renderer != NULL);

  uint64_t outputBufferSize = sizeof(float) * 800 * 800 * 3;
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
    .kernelName = "basic_cuda",
    .kernelMode = KERNEL_MODE_LINEAR,
    .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
    .threadOrganization = {},
    .imageDimensions = {800, 800, 3},
    .pOutputBuffer = pOutputBuffer,
    .outputBufferSize = outputBufferSize,
    .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
    .pModel = pModel,
    .pCamera = pCamera
  };

  renderer->render(&renderProperties);

  for (int x = 0; x < 800 * 800; x += 8 * 3) {
    EXPECT_FLOAT_EQ(((float*)pOutputBuffer)[x + 0], 0.0);
    EXPECT_FLOAT_EQ(((float*)pOutputBuffer)[x + 1], 1.0);
    EXPECT_FLOAT_EQ(((float*)pOutputBuffer)[x + 2], 0.0);
  }

  delete renderer;
  free(pOutputBuffer);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}