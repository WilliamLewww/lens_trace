#include "engine.h"
#include "model.h"
#include "acceleration_structure_optix.h"
#include "camera.h"
#include "structures.h"

int main(int argc, const char** argv) {
  uint64_t outputBufferSize = sizeof(float) * 2048 * 2048 * 3;
  void* pOutputBuffer = malloc(outputBufferSize);

  Camera* pCamera = new Camera(0, 2.5, -50, 0);
  Model* pModel = new Model("cornell_box.obj");

  AccelerationStructureOptix* pAccelerationStructureOptix = new AccelerationStructureOptix(pModel);

  delete pAccelerationStructureOptix;
  delete pModel;
  delete pCamera;
  free(pOutputBuffer);

  return 0;
}