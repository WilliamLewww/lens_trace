#pragma once

#include "tinyobjloader/tiny_obj_loader.h"

#include <unistd.h>
#include <stdint.h>

struct PrimitiveInfo {
  float vertexA[3];
  float vertexB[3];
  float vertexC[3];

  float normalA[3];
  float normalB[3];
  float normalC[3];

  int materialIndex;

  float boundsMin[3];
  float boundsMax[3];
  float centroid[3];
};

struct Material {
  float diffuse[3];
  float ior;
  float dissolve;
};

class Model {
private:
  std::vector<PrimitiveInfo> primitiveInfoList;
  Material* materialBuffer;

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

  std::vector<PrimitiveInfo>* getPrimitiveInfoListP();

  uint64_t getMaterialBufferSize();
  void* getMaterialBuffer();
};