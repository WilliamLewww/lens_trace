#ifdef OPTIX_ENABLED

#pragma once

#include "renderer.h"
#include "acceleration_structure_explicit.h"
#include "camera.h"
#include "structures.h"

#include <stdio.h>
#include <time.h>

class RendererOptiX : public Renderer {
private:
public:
  RendererOptiX();
  ~RendererOptiX();

  void render(void* pRenderProperties);
};

#endif