#include "lens_trace/engine.h"
#include "lens_trace/model.h"
#include "lens_trace/camera.h"
#include "lens_trace/structures.h"
#include "lens_trace/scene_parser.h"

int main(int argc, const char** argv) {
  SceneParser* sceneParser = new SceneParser(argv[1]);
  sceneParser->renderScene();
  delete(sceneParser);

  return 0;
}