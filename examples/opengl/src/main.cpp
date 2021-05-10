#include "lens_trace/model.h"
#include "lens_trace/acceleration_structure_explicit.h"
#include "lens_trace/camera.h"
#include "lens_trace/structures.h"
#include "lens_trace/cuda/renderer_cuda.h"

#include <GLFW/glfw3.h>

int main(int argc, char** argv) {
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow* window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

  // uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  // void* pOutputBuffer = malloc(outputBufferSize);

  // Camera* pCamera = new Camera(0, 2.5, -50, 0);
  // Model* pModel = new Model("resources/models/cornell_box.obj");

  // AccelerationStructureExplicitProperties accelerationStructureExplicitProperties = {
  //   .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
  //   .pNext = NULL,
  //   .accelerationStructureExplicitType = ACCELERATION_STRUCTURE_TYPE_BVH,
  //   .pModel = pModel,
  // };
  // AccelerationStructureExplicit* pAccelerationStructureExplicit = new AccelerationStructureExplicit(accelerationStructureExplicitProperties);

  // RendererCUDA* renderer = new RendererCUDA();
  // RenderPropertiesCUDA renderProperties = {
  //   .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
  //   .pNext = NULL,
  //   .kernelMode = KERNEL_MODE_LINEAR,
  //   .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
  //   .threadOrganization = {},
  //   .imageDimensions = {2048, 2048, 3},
  //   .pOutputBuffer = pOutputBuffer,
  //   .outputBufferSize = outputBufferSize,
  //   .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
  //   .pModel = pModel,
  //   .pCamera = pCamera
  // };

  // renderer->render(&renderProperties);

  // delete pCamera;
  // delete pModel;
  // delete pAccelerationStructureExplicit;
  // free(pOutputBuffer);