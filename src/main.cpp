#include "lens_trace/scene_parser.h"
#include "lens_trace/image_writer.h"

int main(int argc, const char** argv) {
  SceneParser* sceneParser = new SceneParser(argv[1]);

  void* outputBuffer = sceneParser->createOutputBuffer();

  Camera* camera = sceneParser->createCamera();
  Model* model = sceneParser->createModel();
  AccelerationStructureExplicit* accelerationStructureExplicit = sceneParser->createAccelerationStructure(model);
  
  Renderer* renderer;
#ifdef OPENCL_ENABLED
  if (sceneParser->getRenderPlatform() == RENDER_PLATFORM_OPENCL) {
    renderer = new RendererOpenCL();

    RenderPropertiesOpenCL renderPropertiesOpenCL = sceneParser->getRenderPropertiesOpenCL(outputBuffer, accelerationStructureExplicit, model, camera);
    renderer->render(&renderPropertiesOpenCL); 
  }
#endif
#ifdef CUDA_ENABLED
  if (sceneParser->getRenderPlatform() == RENDER_PLATFORM_CUDA) {
    renderer = new RendererCUDA();

    RenderPropertiesCUDA renderPropertiesCUDA = sceneParser->getRenderPropertiesCUDA(outputBuffer, accelerationStructureExplicit, model, camera);
    renderer->render(&renderPropertiesCUDA);
  }
#endif

  BufferToImageProperties bufferToImageProperties = sceneParser->getBufferToImageProperties(outputBuffer);
  ImageWriter::writeBufferToImage(bufferToImageProperties);

  delete(sceneParser);
  return 0;
}