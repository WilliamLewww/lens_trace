#include "engine.h"
#include "model.h"
#include "acceleration_structure_explicit.h"
#include "camera.h"
#include "structures.h"

int main(int argc, const char** argv) {
  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* pOutputBuffer = malloc(outputBufferSize);

  Camera* pCamera = new Camera(0, 2.5, -50, 0);
  Model* pModel = new Model("cornell_box.obj");

  delete pCamera;
  delete pModel;
  free(pOutputBuffer);

  return 0;
}