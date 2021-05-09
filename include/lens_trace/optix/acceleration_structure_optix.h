#ifdef OPTIX_ENABLED

#pragma once
#include "lens_trace/model.h"

class AccelerationStructureOptix {
private:
public:
  AccelerationStructureOptix(Model* pModel);
  ~AccelerationStructureOptix();
};

#endif