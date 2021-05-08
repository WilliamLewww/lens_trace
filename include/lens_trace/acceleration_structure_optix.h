#ifdef OPTIX_ENABLED

#pragma once
#include "model.h"

class AccelerationStructureOptix {
private:
public:
  AccelerationStructureOptix(Model* pModel);
  ~AccelerationStructureOptix();
};

#endif