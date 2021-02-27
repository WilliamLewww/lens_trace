#pragma once

#include "tinyobjloader/tiny_obj_loader.h"

#include <unistd.h>
#include <stdint.h>

class Model {
private:
  std::string fileName;
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warning;
  std::string error;
  bool success;
public:
  Model(std::string fileName);
  ~Model();

  std::string getFileName();

  bool checkError();

  tinyobj::attrib_t getAttrib();
  std::vector<tinyobj::shape_t> getShapes();
};