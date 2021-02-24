
#include "engine.h"

int main(int argc, const char** argv) {
  Engine* engine = new Engine(RENDER_PLATFORM_OPENCL);
  engine->setResolution(2048, 2048, 1);
  engine->render();
  
  delete engine;
  return 0;
}