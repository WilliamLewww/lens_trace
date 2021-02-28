#include "acceleration_structure.h"

AccelerationStructure::AccelerationStructure(AccelerationStructureProperties accelerationStructureProperties) {
  Model* model = (Model*)accelerationStructureProperties.pModel;
  tinyobj::attrib_t attrib = model->getAttrib();
  std::vector<tinyobj::shape_t> shapes = model->getShapes();

  std::vector<std::vector<std::array<float, 3>>> primitiveList;

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
      .index = (int)x,
      .vertexA = {primitiveList[x][0][0], primitiveList[x][1][0], primitiveList[x][2][0]},
      .vertexB = {primitiveList[x][0][1], primitiveList[x][1][1], primitiveList[x][2][1]},
      .vertexC = {primitiveList[x][0][2], primitiveList[x][1][2], primitiveList[x][2][2]},

      .boundsMin = {boundsMinX, boundsMinY, boundsMinZ},
      .boundsMax = {boundsMaxX, boundsMaxY, boundsMaxZ},
      .centroid = {centroidX, centroidY, centroidZ}
    };

    this->primitiveInfoList.push_back(primitiveInfo);
  }

  this->totalNodes = 0;
  BVHBuildNode* root = recursiveBuild(this->primitiveInfoList, 0, primitiveList.size(), &this->totalNodes, this->orderedPrimitiveList);

  this->linearNodes = (LinearBVHNode*)malloc(sizeof(LinearBVHNode) * this->totalNodes);
  int offset = 0;
  flattenBVHTree(this->linearNodes, root, &offset);

  recursiveFree(root);

  int currentVertex = 0;
  this->primitives = (float*)malloc(sizeof(float) * this->orderedPrimitiveList.size() * 3 * 3);
  for (uint64_t x = 0; x < this->orderedPrimitiveList.size(); x++) {
    memcpy(this->primitives + currentVertex + 0, this->orderedPrimitiveList[x]->vertexA, sizeof(float) * 3);
    memcpy(this->primitives + currentVertex + 3, this->orderedPrimitiveList[x]->vertexB, sizeof(float) * 3);
    memcpy(this->primitives + currentVertex + 6, this->orderedPrimitiveList[x]->vertexC, sizeof(float) * 3);
    currentVertex += 9;
  }

  printf("%ld\n", this->totalNodes);
}

AccelerationStructure::~AccelerationStructure() {

}

BVHBuildNode* AccelerationStructure::recursiveBuild(std::vector<PrimitiveInfo>& primitiveInfoList, int start, int end, int* totalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList) {
  BVHBuildNode* node = new BVHBuildNode();
  (*totalNodes) += 1;

  float boundsMin[3];
  float boundsMax[3];
  for (int x = start; x < end; x++) {
    node->boundsMin[0] = std::min(boundsMin[0], primitiveInfoList[x].boundsMin[0]);
    node->boundsMin[1] = std::min(boundsMin[1], primitiveInfoList[x].boundsMin[1]);
    node->boundsMin[2] = std::min(boundsMin[2], primitiveInfoList[x].boundsMin[2]);

    node->boundsMax[0] = std::max(boundsMax[0], primitiveInfoList[x].boundsMax[0]);
    node->boundsMax[1] = std::max(boundsMax[1], primitiveInfoList[x].boundsMax[1]);
    node->boundsMax[2] = std::max(boundsMax[2], primitiveInfoList[x].boundsMax[2]);
  }

  int primitiveCount = end - start;
  if (primitiveCount == 1) {
    int firstPrimitiveOffset = orderedPrimitiveList.size();
    for (int x = start; x < end; x++) {
      orderedPrimitiveList.push_back(&primitiveInfoList[x]);
    }

    node->leftChild = NULL;
    node->rightChild = NULL;

    node->splitAxis = -1;
    node->firstPrimitiveOffset = firstPrimitiveOffset;
    node->primitiveCount = primitiveCount;
    return node;
  }
  else {
    float centroidBoundsMin[3];
    float centroidBoundsMax[3];
    for (int x = start; x < end; x++) {
      centroidBoundsMin[0] = std::min(centroidBoundsMin[0], primitiveInfoList[x].centroid[0]);
      centroidBoundsMin[1] = std::min(centroidBoundsMin[1], primitiveInfoList[x].centroid[1]);
      centroidBoundsMin[2] = std::min(centroidBoundsMin[2], primitiveInfoList[x].centroid[2]);

      centroidBoundsMax[0] = std::max(centroidBoundsMax[0], primitiveInfoList[x].centroid[0]);
      centroidBoundsMax[1] = std::max(centroidBoundsMax[1], primitiveInfoList[x].centroid[1]);
      centroidBoundsMax[2] = std::max(centroidBoundsMax[2], primitiveInfoList[x].centroid[2]);
    }
    int dim = -1;
    float diagonal[3] = {centroidBoundsMax[0] - centroidBoundsMin[0], centroidBoundsMax[1] - centroidBoundsMin[1], centroidBoundsMax[2] - centroidBoundsMin[2]};
    if (diagonal[0] > diagonal[1] && diagonal[0] > diagonal[2]) {
      dim = 0;
    }
    else {
      if (diagonal[1] > diagonal[2]) {
        dim = 1;
      }
      else {
        dim = 2;
      }
    }

    int mid = (start + end) / 2;
    if (centroidBoundsMax[dim] == centroidBoundsMin[dim]) {
      int firstPrimitiveOffset = orderedPrimitiveList.size();
      for (int x = start; x < end; x++) {
        orderedPrimitiveList.push_back(&primitiveInfoList[x]);
      }

      node->leftChild = NULL;
      node->rightChild = NULL;

      node->splitAxis = -1;
      node->firstPrimitiveOffset = firstPrimitiveOffset;
      node->primitiveCount = primitiveCount;
      return node;
    }
    else {
      mid = (start + end) / 2;
      std::nth_element(&primitiveInfoList[start], &primitiveInfoList[mid], &primitiveInfoList[end - 1] + 1, [dim](const PrimitiveInfo& a, const PrimitiveInfo& b) {
        return a.centroid[dim] < b.centroid[dim];
      });

      node->splitAxis = dim;
      node->firstPrimitiveOffset = -1;
      node->primitiveCount = 0;

      node->leftChild = recursiveBuild(primitiveInfoList, start, mid, totalNodes, orderedPrimitiveList);
      node->rightChild = recursiveBuild(primitiveInfoList, mid, end, totalNodes, orderedPrimitiveList);
    }
  }

  return node;
}

void AccelerationStructure::recursiveFree(BVHBuildNode* node) {
  if (node == NULL) {
    return;
  }

  recursiveFree(node->leftChild);
  recursiveFree(node->rightChild);

  delete node;
}

int AccelerationStructure::flattenBVHTree(LinearBVHNode* linearBVHNodes, BVHBuildNode* node, int* offset) {
  LinearBVHNode* linearNode = &linearBVHNodes[*offset];
  memcpy(linearNode->boundsMin, node->boundsMin, sizeof(float) * 3);
  memcpy(linearNode->boundsMax, node->boundsMax, sizeof(float) * 3);

  int currentOffset = (*offset)++;

  if (node->primitiveCount > 0) {
    linearNode->primitivesOffset = node->firstPrimitiveOffset;
    linearNode->primitiveCount = node->primitiveCount;
  }
  else {
    linearNode->axis = node->splitAxis;
    linearNode->primitiveCount = 0;
    flattenBVHTree(linearBVHNodes, node->leftChild, offset);
    linearNode->secondChildOffset = flattenBVHTree(linearBVHNodes, node->rightChild, offset);
  }

  return currentOffset;
}

uint64_t AccelerationStructure::getNodeBufferSize() {
  return sizeof(LinearBVHNode) * this->totalNodes;
}

LinearBVHNode* AccelerationStructure::getNodeBuffer() {
  return this->linearNodes;
}

uint64_t AccelerationStructure::getPrimitiveBufferSize() {
  return sizeof(float) * this->orderedPrimitiveList.size() * 3 * 3;
}

float* AccelerationStructure::getPrimitiveBuffer() {
  return this->primitives;
}