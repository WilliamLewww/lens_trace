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

TEST (RenderBufferTEST, CustomBlockSize) {
  RendererCUDA* renderer = new RendererCUDA();
  EXPECT_TRUE(renderer != NULL);

  uint64_t outputBufferSize = sizeof(float) * 100 * 100 * 3;
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
    .kernelName = "basic_cuda",
    .kernelMode = KERNEL_MODE_LINEAR,
    .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
    .threadOrganization = {},
    .imageDimensions = {100, 100, 3},
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
  renderProperties.threadOrganization = threadOrganization;

  renderer->render(&renderProperties);

  renderProperties.pOutputBuffer = pOutputBufferC;
  threadOrganization.blockSize[0] = 4;
  threadOrganization.blockSize[1] = 4;
  renderProperties.threadOrganization = threadOrganization;

  renderer->render(&renderProperties);

  for (int x = 0; x < 100 * 100; x += 32) {
    EXPECT_FLOAT_EQ(((float*)pOutputBufferA)[x], ((float*)pOutputBufferB)[x]);
    EXPECT_FLOAT_EQ(((float*)pOutputBufferB)[x], ((float*)pOutputBufferC)[x]);
  }

  delete renderer;
}

TEST (RenderBufferTEST, KernelMode) {
  RendererCUDA* renderer = new RendererCUDA();
  EXPECT_TRUE(renderer != NULL);

  uint64_t outputBufferSize = sizeof(float) * 800 * 800 * 3;
  void* pOutputBufferA = malloc(outputBufferSize);
  void* pOutputBufferB = malloc(outputBufferSize);

  Camera* pCamera = new Camera(0, 2.5, -50, 0);
  Model* pModel = new Model("resources/models/green_wall.obj");

  AccelerationStructureExplicitProperties accelerationStructureExplicitProperties = {
    .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
    .pNext = NULL,
    .accelerationStructureExplicitType = ACCELERATION_STRUCTURE_TYPE_BVH,
    .pModel = pModel,
  };
  AccelerationStructureExplicit* pAccelerationStructureExplicit = new AccelerationStructureExplicit(accelerationStructureExplicitProperties);

  {
    RenderPropertiesCUDA renderProperties = {
      .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
      .pNext = NULL,
      .kernelName = "basic_cuda",
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
  }
  {
    RenderPropertiesCUDA renderProperties = {
      .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
      .pNext = NULL,
      .kernelName = "basic_cuda",
      .kernelMode = KERNEL_MODE_TILE,
      .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
      .threadOrganization = {},
      .imageDimensions = {800, 800, 3},
      .pOutputBuffer = pOutputBufferB,
      .outputBufferSize = outputBufferSize,
      .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
      .pModel = pModel,
      .pCamera = pCamera
    };

    renderer->render(&renderProperties);
  }

  for (int x = 0; x < 800 * 800 * 3; x += 32) {
    EXPECT_FLOAT_EQ(((float*)pOutputBufferA)[x], ((float*)pOutputBufferB)[x]);
  }

  delete renderer;
  free(pOutputBufferB);
  free(pOutputBufferA);
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