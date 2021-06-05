#include <gtest/gtest.h>

#include "lens_trace/opencl/renderer_opencl.h"

TEST (CreateEngineTEST, ValidEngine) {
  RendererOpenCL* renderer = new RendererOpenCL();
  EXPECT_TRUE(renderer != NULL);

  delete renderer;
}

TEST (RenderBufferTEST, ValidBuffer) {
  RendererOpenCL* renderer = new RendererOpenCL();
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

  RenderPropertiesOpenCL renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
    .pNext = NULL,
    .kernelFilePath = "resources/kernels/basic_opencl.kernel",
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
}

TEST (RenderBufferTEST, CustomBlockSize) {
  RendererOpenCL* renderer = new RendererOpenCL();
  EXPECT_TRUE(renderer != NULL);

  uint64_t outputBufferSize = sizeof(float) * 800 * 800 * 3;
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

  RenderPropertiesOpenCL renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
    .pNext = NULL,
    .kernelFilePath = "resources/kernels/basic_opencl.kernel",
    .kernelMode = KERNEL_MODE_LINEAR,
    .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
    .threadOrganization = {},
    .imageDimensions = {800, 800, 3},
    .pOutputBuffer = pOutputBufferA,
    .outputBufferSize = outputBufferSize,
    .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
    .pModel = pModel,
    .pCamera = pCamera
  };

  renderer->render(&renderProperties);

  renderProperties.pOutputBuffer = pOutputBufferB;
  renderProperties.threadOrganizationMode = THREAD_ORGANIZATION_MODE_CUSTOM;
  ThreadOrganizationOpenCL threadOrganization = {
    .sType = STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL,
    .pNext = NULL,
    .workBlockSize = {16, 16},
    .threadGroupSize = {16, 16}
  };
  renderProperties.threadOrganization = threadOrganization;

  renderer->render(&renderProperties);

  renderProperties.pOutputBuffer = pOutputBufferC;
  threadOrganization.workBlockSize[0] = 8;
  threadOrganization.workBlockSize[1] = 8;
  threadOrganization.threadGroupSize[0] = 8;
  threadOrganization.threadGroupSize[1] = 8;
  renderProperties.threadOrganization = threadOrganization;

  renderer->render(&renderProperties);

  for (int x = 0; x < 800 * 800; x += 32) {
    EXPECT_FLOAT_EQ(((float*)pOutputBufferA)[x], ((float*)pOutputBufferB)[x]);
    EXPECT_FLOAT_EQ(((float*)pOutputBufferB)[x], ((float*)pOutputBufferC)[x]);
  }

  delete renderer;
}

TEST (RenderBufferTEST, CorrectColor) {
  RendererOpenCL* renderer = new RendererOpenCL();
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

  RenderPropertiesOpenCL renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
    .pNext = NULL,
    .kernelFilePath = "resources/kernels/basic_opencl.kernel",
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
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}