#include "lens_trace/model.h"
#include "lens_trace/acceleration_structure_explicit.h"
#include "lens_trace/camera.h"
#include "lens_trace/structures.h"
#include "lens_trace/cuda/renderer_cuda.h"

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

std::vector<int> keyDownList;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    keyDownList.push_back(key);
  }
  if (action == GLFW_RELEASE) {
    keyDownList.erase(std::remove(keyDownList.begin(), keyDownList.end(), key), keyDownList.end());
  }
}

bool checkKeyDown(int key) {
  return std::find(keyDownList.begin(), keyDownList.end(), key) != keyDownList.end();
}

int main(int argc, char** argv) {
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow* window = glfwCreateWindow(800, 800, "OpenGL Example", NULL, NULL);
  glfwSetKeyCallback(window, keyCallback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glewInit();

  FILE* vertexFile = fopen("resources/shaders/basic.vert", "rb");
  fseek(vertexFile, 0, SEEK_END);
  uint32_t vertexFileSize = ftell(vertexFile);
  fseek(vertexFile, 0, SEEK_SET);

  char* vertexFileBuffer = (char*)malloc(vertexFileSize + 1);
  fread(vertexFileBuffer, 1, vertexFileSize, vertexFile);
  fclose(vertexFile);
  vertexFileBuffer[vertexFileSize] = '\0';

  FILE* fragmentFile = fopen("resources/shaders/basic.frag", "rb");
  fseek(fragmentFile, 0, SEEK_END);
  uint32_t fragmentFileSize = ftell(fragmentFile);
  fseek(fragmentFile, 0, SEEK_SET);

  char* fragmentFileBuffer = (char*)malloc(fragmentFileSize + 1);
  fread(fragmentFileBuffer, 1, fragmentFileSize, fragmentFile);
  fclose(fragmentFile);
  fragmentFileBuffer[fragmentFileSize] = '\0';

  GLuint vertexBuffer;
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint program;
    
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexFileBuffer, NULL);
  glCompileShader(vertexShader);

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentFileBuffer, NULL);
  glCompileShader(fragmentShader);

  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  float vertexPositions[12] = {
    -1, -1,
    -1,  1,
    1,   1,
    -1, -1,
    1,   1,
    1,  -1
  };

  float texturePositions[12] = {
    0, 1,
    0, 0,
    1, 0,
    0, 1,
    1, 0,
    1, 1
  };

  GLuint vao[1];
  GLuint vbo[2];

  glGenVertexArrays(1, vao);
  glBindVertexArray(vao[0]);
  glGenBuffers(2, vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texturePositions), texturePositions, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* pOutputBuffer = malloc(outputBufferSize);

  Camera* pCamera = new Camera(0, 2.5, -50, 0);
  Model* pModel = new Model("resources/models/cornell_box.obj");

  AccelerationStructureExplicitProperties accelerationStructureExplicitProperties = {
    .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
    .pNext = NULL,
    .accelerationStructureExplicitType = ACCELERATION_STRUCTURE_TYPE_BVH,
    .pModel = pModel,
  };
  AccelerationStructureExplicit* pAccelerationStructureExplicit = new AccelerationStructureExplicit(accelerationStructureExplicitProperties);

  RendererCUDA* renderer = new RendererCUDA();
  RenderPropertiesCUDA renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
    .pNext = NULL,
    .kernelName = "basic_cuda",
    .kernelMode = KERNEL_MODE_LINEAR,
    .threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT,
    .threadOrganization = {},
    .imageDimensions = {2048, 2048, 3},
    .pOutputBuffer = pOutputBuffer,
    .outputBufferSize = outputBufferSize,
    .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
    .pModel = pModel,
    .pCamera = pCamera
  };

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_FLOAT, pOutputBuffer);

  while (!glfwWindowShouldClose(window)) {
    float positionX = 0.0;
    float positionZ = 0.0;
    float yaw = 0.0;
    if (checkKeyDown(GLFW_KEY_W)) {
      positionX -= cos(-pCamera->getYaw() - (M_PI / 2)) * 0.1f;
      positionZ -= sin(-pCamera->getYaw() - (M_PI / 2)) * 0.1f;
    }
    if (checkKeyDown(GLFW_KEY_S)) {
      positionX += cos(-pCamera->getYaw() - (M_PI / 2)) * 0.1f;
      positionZ += sin(-pCamera->getYaw() - (M_PI / 2)) * 0.1f;
    }
    if (checkKeyDown(GLFW_KEY_A)) {
      positionX += cos(-pCamera->getYaw()) * 0.1f;
      positionZ += sin(-pCamera->getYaw()) * 0.1f;
    }
    if (checkKeyDown(GLFW_KEY_D)) {
      positionX -= cos(-pCamera->getYaw()) * 0.1f;
      positionZ -= sin(-pCamera->getYaw()) * 0.1f;
    }
    if (checkKeyDown(GLFW_KEY_Q)) {
      yaw += 0.005f;
    }
    if (checkKeyDown(GLFW_KEY_E)) {
      yaw -= 0.005f;
    }

    if (positionX != 0 || positionZ != 0) {
      pCamera->updatePosition(positionX, 0, positionZ);
    }
    if (yaw != 0) {
      pCamera->updateRotation(yaw, 0, 0);
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    renderer->render(&renderProperties);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2048, 2048, GL_RGB, GL_FLOAT, pOutputBuffer);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    glUniform1i(glGetUniformLocation(program, "texture"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(vao[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  delete pCamera;
  delete pModel;
  delete pAccelerationStructureExplicit;
  free(pOutputBuffer);

  return 0;
}