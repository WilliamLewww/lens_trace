#pragma once

#include "stb/stb_image_write.h"

#include "lens_trace/structures.h"

class ImageWriter {
public:
  static void writeBufferToImage(BufferToImageProperties bufferToImageProperties);
};