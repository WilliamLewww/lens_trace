#include <gtest/gtest.h>

#include "renderer_opencl.h"

TEST (CreateEngineTEST, ValidEngine) {
  RendererOpenCL* renderer = new RendererOpenCL();
  EXPECT_TRUE(renderer != NULL);

  delete renderer;
}

TEST (RenderBufferTEST, ValidBuffer) {
  RendererOpenCL* renderer = new RendererOpenCL();
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

  renderer->render(&renderProperties);

  // for (int x = 0; x < 2048 * 2048; x += 32) {
  //   EXPECT_FLOAT_EQ(0.0, ((float*)pOutputBuffer)[(x * 3) + 0]);
  //   EXPECT_FLOAT_EQ(1.0, ((float*)pOutputBuffer)[(x * 3) + 1]);
  //   EXPECT_FLOAT_EQ(0.0, ((float*)pOutputBuffer)[(x * 3) + 2]);
  // }

  delete renderer;
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}