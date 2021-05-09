#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#include "stb/stb_image_write.h"

#include "lens_trace/scene_parser.h"

void writeBufferToImage(BufferToImageProperties bufferToImageProperties) {
  if (bufferToImageProperties.sType == STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES) {
    float* pImageBuffer = (float*)bufferToImageProperties.pBuffer;
    char* pWriteBuffer = (char*)malloc(bufferToImageProperties.bufferSize);

    uint64_t imageWidth = bufferToImageProperties.imageDimensions[0];
    uint64_t imageHeight = bufferToImageProperties.imageDimensions[1];
    uint64_t imageDepth = bufferToImageProperties.imageDimensions[2];

    for (int x = 0; x < imageWidth * imageHeight * imageDepth; x++) {
      // pWriteBuffer[(imageWidth * imageHeight * (x % 3)) + (x / 3)] = pImageBuffer[x];
      pWriteBuffer[x] = pImageBuffer[x] * 255;
    }

    stbi_write_jpg(bufferToImageProperties.filename, imageWidth, imageHeight, imageDepth, pWriteBuffer, 100);
    free(pWriteBuffer);
  }
}

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
  writeBufferToImage(bufferToImageProperties);

  delete(sceneParser);
  return 0;
}