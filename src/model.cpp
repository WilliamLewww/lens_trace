#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif

#include "lens_trace/model.h"

Model::Model(std::string fileName) {
  this->fileName = Resource::findResource(fileName);
  this->success = tinyobj::LoadObj(&this->attrib, &this->shapes, &this->materials, &this->warning, &this->error, this->fileName.c_str());

  this->checkError();

  std::vector<std::vector<std::array<float, 3>>> facePositionList;
  std::vector<std::vector<std::array<float, 3>>> faceNormalList;
  std::vector<int> faceMaterialIndexList;

  for (uint64_t s = 0; s < this->shapes.size(); s++) {
    uint64_t index_offset = 0;
    for (uint64_t f = 0; f < this->shapes[s].mesh.num_face_vertices.size(); f++) {
      int fv = this->shapes[s].mesh.num_face_vertices[f];

      std::vector<std::array<float, 3>> vertexPositionList;
      std::vector<std::array<float, 3>> vertexNormalList;
      for (uint64_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = this->shapes[s].mesh.indices[index_offset + v];
        vertexPositionList.push_back({this->attrib.vertices[3*idx.vertex_index+0], this->attrib.vertices[3*idx.vertex_index+1], this->attrib.vertices[3*idx.vertex_index+2]});
        vertexNormalList.push_back({this->attrib.normals[3*idx.normal_index+0], this->attrib.normals[3*idx.normal_index+1], this->attrib.normals[3*idx.normal_index+2]});
      }
      facePositionList.push_back(vertexPositionList);
      faceNormalList.push_back(vertexNormalList);
      faceMaterialIndexList.push_back(shapes[s].mesh.material_ids[f]);

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
      .positionA = {facePositionList[x][0][0], facePositionList[x][0][1], facePositionList[x][0][2]},
      .positionB = {facePositionList[x][1][0], facePositionList[x][1][1], facePositionList[x][1][2]},
      .positionC = {facePositionList[x][2][0], facePositionList[x][2][1], facePositionList[x][2][2]},

      .normalA = {faceNormalList[x][0][0], faceNormalList[x][0][1], faceNormalList[x][0][2]},
      .normalB = {faceNormalList[x][1][0], faceNormalList[x][1][1], faceNormalList[x][1][2]},
      .normalC = {faceNormalList[x][2][0], faceNormalList[x][2][1], faceNormalList[x][2][2]},

      .materialIndex = faceMaterialIndexList[x],

      .boundsMin = {boundsMin[0], boundsMin[1], boundsMin[2]},
      .boundsMax = {boundsMax[0], boundsMax[1], boundsMax[2]},
      .centroid = {centroid[0], centroid[1], centroid[2]}
    };

    this->primitiveInfoList.push_back(primitiveInfo);
  }

  this->materialBuffer = (Material*)malloc(sizeof(Material) * this->materials.size());
  for (int x = 0; x < this->materials.size(); x++) {
    memcpy(this->materialBuffer[x].diffuse, this->materials[x].diffuse, sizeof(float) * 3);
    memcpy(&this->materialBuffer[x].ior, &this->materials[x].ior, sizeof(float));
    memcpy(&this->materialBuffer[x].dissolve, &this->materials[x].dissolve, sizeof(float));
    memcpy(this->materialBuffer[x].emission, this->materials[x].emission, sizeof(float) * 3);
  }
}

Model::~Model() {
  free(this->materialBuffer);
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

uint64_t Model::getMaterialBufferSize() {
  return sizeof(Material) * this->materials.size();
}

void* Model::getMaterialBuffer() {
  return this->materialBuffer;
}

float* Model::getVertices() {
  return this->attrib.vertices.data();
}

uint32_t Model::getVertexCount() {
  return this->attrib.vertices.size();
}

tinyobj::index_t Model::getIndex(uint32_t index) {
  for (int x = 0; x < this->shapes.size(); x++) {
    if (index >= this->shapes[x].mesh.indices.size()) {
      index -= this->shapes[x].mesh.indices.size();
    }
    else {
      return this->shapes[x].mesh.indices[index];
    }
  }

  return {-1, -1, -1};
}

uint32_t Model::getIndexCount() {
  uint32_t indexCount = 0;
  for (int x = 0; x < this->shapes.size(); x++) {
    indexCount += this->shapes[x].mesh.indices.size();
  }

  return indexCount;
}