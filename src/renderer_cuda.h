#pragma once

#include "renderer.h"
#include "acceleration_structure.h"
#include "camera.h"
#include "structures.h"

#include <stdio.h>
#include <time.h>

class RendererCUDA : public Renderer {
private:
public:
  RendererCUDA();
  ~RendererCUDA();

  void render(void* pRenderProperties);
};