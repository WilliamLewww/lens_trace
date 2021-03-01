#include "acceleration_structure.h"

AccelerationStructure::AccelerationStructure(AccelerationStructureProperties accelerationStructureProperties) {
  Model* model = (Model*)accelerationStructureProperties.pModel;

  this->totalPrimitives = model->getPrimitiveInfoListP()->size();
  this->totalNodes = 0;
  std::vector<PrimitiveInfo*> orderedPrimitiveList;
  BVHBuildNode* root = recursiveBuild(model->getPrimitiveInfoListP(), 0, model->getPrimitiveInfoListP()->size(), &this->totalNodes, orderedPrimitiveList);

  int offset = 0;
  this->linearNodeBuffer = (LinearBVHNode*)malloc(sizeof(LinearBVHNode) * this->totalNodes);
  flattenBVHTree(this->linearNodeBuffer, root, &offset);
  recursiveFree(root);

  int currentVertex = 0;
  this->orderedVertexBuffer = (float*)malloc(sizeof(float) * orderedPrimitiveList.size() * 3 * 3);
  for (uint64_t x = 0; x < orderedPrimitiveList.size(); x++) {
    memcpy(this->orderedVertexBuffer + currentVertex + 0, orderedPrimitiveList[x]->vertexA, sizeof(float) * 3);
    memcpy(this->orderedVertexBuffer + currentVertex + 3, orderedPrimitiveList[x]->vertexB, sizeof(float) * 3);
    memcpy(this->orderedVertexBuffer + currentVertex + 6, orderedPrimitiveList[x]->vertexC, sizeof(float) * 3);
    currentVertex += 9;
  }
}

AccelerationStructure::~AccelerationStructure() {

}

BVHBuildNode* AccelerationStructure::recursiveBuild(std::vector<PrimitiveInfo>* pPrimitiveInfoList, int start, int end, int* totalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList) {
  std::vector<PrimitiveInfo>& primitiveInfoList = *pPrimitiveInfoList;

  BVHBuildNode* node = new BVHBuildNode();
  (*totalNodes) += 1;

  memcpy(node->boundsMin, primitiveInfoList[start].boundsMin, sizeof(float) * 3);
  memcpy(node->boundsMax, primitiveInfoList[start].boundsMax, sizeof(float) * 3);
  for (int x = start; x < end; x++) {
    node->boundsMin[0] = std::min(node->boundsMin[0], primitiveInfoList[x].boundsMin[0]);
    node->boundsMin[1] = std::min(node->boundsMin[1], primitiveInfoList[x].boundsMin[1]);
    node->boundsMin[2] = std::min(node->boundsMin[2], primitiveInfoList[x].boundsMin[2]);

    node->boundsMax[0] = std::max(node->boundsMax[0], primitiveInfoList[x].boundsMax[0]);
    node->boundsMax[1] = std::max(node->boundsMax[1], primitiveInfoList[x].boundsMax[1]);
    node->boundsMax[2] = std::max(node->boundsMax[2], primitiveInfoList[x].boundsMax[2]);
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

      node->leftChild = recursiveBuild(pPrimitiveInfoList, start, mid, totalNodes, orderedPrimitiveList);
      node->rightChild = recursiveBuild(pPrimitiveInfoList, mid, end, totalNodes, orderedPrimitiveList);
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

void* AccelerationStructure::getNodeBuffer() {
  return this->linearNodeBuffer;
}

uint64_t AccelerationStructure::getOrderedVertexBufferSize() {
  return sizeof(float) * this->totalPrimitives * 3 * 3;
}

void* AccelerationStructure::getOrderedVertexBuffer() {
  return this->orderedVertexBuffer;
}