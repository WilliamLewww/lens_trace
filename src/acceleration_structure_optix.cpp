#include "acceleration_structure_optix.h"

extern "C" {
  void createAccelerationStructure(void* vertexBuffer, uint32_t vertexCount, void* indexBuffer, uint32_t indexCount);
}

AccelerationStructureOptix::AccelerationStructureOptix() {

}

AccelerationStructureOptix::~AccelerationStructureOptix() {

}