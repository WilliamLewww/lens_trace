#ifdef OPTIX_ENABLED

#include "lens_trace/optix/acceleration_structure_optix.h"

extern "C" {
  void createAccelerationStructure(void* vertexBuffer, uint32_t vertexCount, void* indexBuffer, uint32_t indexCount);
}

AccelerationStructureOptix::AccelerationStructureOptix(Model* pModel) {
  uint32_t indexCount = pModel->getIndexCount();

  uint32_t* indexBuffer = (uint32_t*)malloc(sizeof(uint32_t) * indexCount);
  for (int x = 0; x < indexCount; x++) {
    indexBuffer[x] = pModel->getIndex(x).vertex_index;
  }

  createAccelerationStructure(pModel->getVertices(), pModel->getVertexCount(), indexBuffer, indexCount);
}

AccelerationStructureOptix::~AccelerationStructureOptix() {

}

#endif