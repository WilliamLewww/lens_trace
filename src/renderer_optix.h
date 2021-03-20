#pragma once

#include "renderer.h"
#include "acceleration_structure.h"
#include "camera.h"
#include "structures.h"

#include <stdio.h>
#include <time.h>

class RendererOptix : public Renderer {
private:
public:
  RendererOptix();
  ~RendererOptix();

  void render(void* pRenderProperties);
};