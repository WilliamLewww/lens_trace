#include "engine.h"
#include "model.h"
#include "acceleration_structure_optix.h"
#include "camera.h"
#include "structures.h"
#include "scene_parser.h"

int main(int argc, const char** argv) {
  SceneParser* sceneParser = new SceneParser("resources/scenes/basic.scene");
  sceneParser->renderScene();
  delete(sceneParser);

  return 0;
}