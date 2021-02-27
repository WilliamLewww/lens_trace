#include "acceleration_structure.h"

struct PrimitiveInfo {
  uint64_t primitiveNumber;
  float boundsMin[3];
  float boundsMax[3];
  float centroid[3];
};

AccelerationStructure::AccelerationStructure(Model* model) {
  tinyobj::attrib_t attrib = model->getAttrib();
  std::vector<tinyobj::shape_t> shapes = model->getShapes();

  std::vector<std::vector<std::array<float, 3>>> primitiveList;
  std::vector<PrimitiveInfo> primitiveInfoList;

  for (uint64_t s = 0; s < shapes.size(); s++) {
    uint64_t index_offset = 0;
    for (uint64_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      int fv = shapes[s].mesh.num_face_vertices[f];

      std::vector<std::array<float, 3>> vertexList;
      for (uint64_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        vertexList.push_back({attrib.vertices[3*idx.vertex_index+0], attrib.vertices[3*idx.vertex_index+1], attrib.vertices[3*idx.vertex_index+2]});
      }
      primitiveList.push_back(vertexList);

      index_offset += fv;
    }
  }

  for (uint64_t x = 0; x < primitiveList.size(); x++) {
    float boundsMinX = std::min(std::min(primitiveList[x][0][0], primitiveList[x][1][0]), primitiveList[x][2][0]);
    float boundsMinY = std::min(std::min(primitiveList[x][0][1], primitiveList[x][1][1]), primitiveList[x][2][1]);
    float boundsMinZ = std::min(std::min(primitiveList[x][0][2], primitiveList[x][1][2]), primitiveList[x][2][2]);

    float boundsMaxX = std::max(std::max(primitiveList[x][0][0], primitiveList[x][1][0]), primitiveList[x][2][0]);
    float boundsMaxY = std::max(std::max(primitiveList[x][0][1], primitiveList[x][1][1]), primitiveList[x][2][1]);
    float boundsMaxZ = std::max(std::max(primitiveList[x][0][2], primitiveList[x][1][2]), primitiveList[x][2][2]);

    float centroidX = 0.5 * boundsMinX + 0.5 * boundsMaxX;
    float centroidY = 0.5 * boundsMinY + 0.5 * boundsMaxY;
    float centroidZ = 0.5 * boundsMinZ + 0.5 * boundsMaxZ;

    PrimitiveInfo primitiveInfo = {
      .primitiveNumber = x,
      .boundsMin = {boundsMinX, boundsMinY, boundsMinZ},
      .boundsMax = {boundsMaxX, boundsMaxY, boundsMaxZ},
      .centroid = {centroidX, centroidY, centroidZ}
    };

    primitiveInfoList.push_back(primitiveInfo);
  }
}

AccelerationStructure::~AccelerationStructure() {

}