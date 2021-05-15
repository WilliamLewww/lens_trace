#pragma once

#include "tinyobjloader/tiny_obj_loader.h"

#include "lens_trace/resource.h"

#include <unistd.h>
#include <stdint.h>

struct PrimitiveInfo {
  float positionA[3];
  float positionB[3];
  float positionC[3];

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
  float emission[3];
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

  float* getVertices();
  uint32_t getVertexCount();

  tinyobj::index_t getIndex(uint32_t index);
  uint32_t getIndexCount();
};