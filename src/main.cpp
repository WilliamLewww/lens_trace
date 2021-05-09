#include "lens_trace/model.h"
#include "lens_trace/camera.h"
#include "lens_trace/structures.h"
#include "lens_trace/scene_parser.h"

int main(int argc, const char** argv) {
  SceneParser* sceneParser = new SceneParser(argv[1]);
  delete(sceneParser);

  return 0;
}

// void Engine::writeBufferToImage(BufferToImageProperties bufferToImageProperties) {
//   if (bufferToImageProperties.sType == STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES) {
//     float* pImageBuffer = (float*)bufferToImageProperties.pBuffer;
//     char* pWriteBuffer = (char*)malloc(bufferToImageProperties.bufferSize);

//     uint64_t imageWidth = bufferToImageProperties.imageDimensions[0];
//     uint64_t imageHeight = bufferToImageProperties.imageDimensions[1];
//     uint64_t imageDepth = bufferToImageProperties.imageDimensions[2];

//     for (int x = 0; x < imageWidth * imageHeight * imageDepth; x++) {
//       // pWriteBuffer[(imageWidth * imageHeight * (x % 3)) + (x / 3)] = pImageBuffer[x];
//       pWriteBuffer[x] = pImageBuffer[x] * 255;
//     }

//     stbi_write_jpg(bufferToImageProperties.filename, imageWidth, imageHeight, imageDepth, pWriteBuffer, 100);
//     free(pWriteBuffer);
//   }
// }