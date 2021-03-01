#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif

#include "model.h"

Model::Model(std::string fileName) {
  chdir("res");

  this->fileName = fileName;
  this->success = tinyobj::LoadObj(&this->attrib, &this->shapes, &this->materials, &this->warning, &this->error, fileName.c_str());

  chdir("..");

  this->checkError();

  std::vector<std::vector<std::array<float, 3>>> facePositionList;

  for (uint64_t s = 0; s < this->shapes.size(); s++) {
    uint64_t index_offset = 0;
    for (uint64_t f = 0; f < this->shapes[s].mesh.num_face_vertices.size(); f++) {
      int fv = this->shapes[s].mesh.num_face_vertices[f];

      std::vector<std::array<float, 3>> vertexPositionList;
      for (uint64_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = this->shapes[s].mesh.indices[index_offset + v];
        vertexPositionList.push_back({this->attrib.vertices[3*idx.vertex_index+0], this->attrib.vertices[3*idx.vertex_index+1], this->attrib.vertices[3*idx.vertex_index+2]});
      }
      facePositionList.push_back(vertexPositionList);

      index_offset += fv;
    }
  }

  for (uint64_t x = 0; x < facePositionList.size(); x++) {
    float boundsMin[3] = {
      std::min(std::min(facePositionList[x][0][0], facePositionList[x][1][0]), facePositionList[x][2][0]),
      std::min(std::min(facePositionList[x][0][1], facePositionList[x][1][1]), facePositionList[x][2][1]),
      std::min(std::min(facePositionList[x][0][2], facePositionList[x][1][2]), facePositionList[x][2][2])
    };

    float boundsMax[3] = {
      std::max(std::max(facePositionList[x][0][0], facePositionList[x][1][0]), facePositionList[x][2][0]),
      std::max(std::max(facePositionList[x][0][1], facePositionList[x][1][1]), facePositionList[x][2][1]),
      std::max(std::max(facePositionList[x][0][2], facePositionList[x][1][2]), facePositionList[x][2][2])
    };

    float centroid[3] = {
      0.5f * boundsMin[0] + 0.5f * boundsMax[0],
      0.5f * boundsMin[1] + 0.5f * boundsMax[1],
      0.5f * boundsMin[2] + 0.5f * boundsMax[2]
    };

    PrimitiveInfo primitiveInfo = {
      .vertexA = {facePositionList[x][0][0], facePositionList[x][0][1], facePositionList[x][0][2]},
      .vertexB = {facePositionList[x][1][0], facePositionList[x][1][1], facePositionList[x][1][2]},
      .vertexC = {facePositionList[x][2][0], facePositionList[x][2][1], facePositionList[x][2][2]},

      .boundsMin = {boundsMin[0], boundsMin[1], boundsMin[2]},
      .boundsMax = {boundsMax[0], boundsMax[1], boundsMax[2]},
      .centroid = {centroid[0], centroid[1], centroid[2]}
    };

    this->primitiveInfoList.push_back(primitiveInfo);
  }
}

Model::~Model() {

}

std::string Model::getFileName() {
  return this->fileName;
}

bool Model::checkError() {
  if (!this->warning.empty()) {
    printf("%s\n", this->warning.c_str());
  }

  if (!this->error.empty()) {
    printf("%s\n", this->error.c_str());
  }

  return this->success;
}

tinyobj::attrib_t Model::getAttrib() {
  return this->attrib;
}

std::vector<tinyobj::shape_t> Model::getShapes() {
  return this->shapes;
}

std::vector<PrimitiveInfo>* Model::getPrimitiveInfoListP() {
  return &this->primitiveInfoList;
}