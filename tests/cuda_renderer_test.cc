#include <gtest/gtest.h>

#include "renderer_cuda.h"

TEST (CreateEngineTEST, ValidEngine) {
  RendererCUDA* renderer = new RendererCUDA();
  EXPECT_TRUE(renderer != NULL);

  delete renderer;
}

TEST (RenderBufferTEST, ValidBuffer) {
  RendererCUDA* renderer = new RendererCUDA();
  EXPECT_TRUE(renderer != NULL);

  delete renderer;
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}