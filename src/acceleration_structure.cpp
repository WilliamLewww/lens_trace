#include "acceleration_structure.h"

AccelerationStructure::AccelerationStructure(AccelerationStructureProperties accelerationStructureProperties) {
  Model* pModel = (Model*)accelerationStructureProperties.pModel;

  this->totalPrimitives = pModel->getPrimitiveInfoListP()->size();
  this->totalNodes = 0;
  std::vector<PrimitiveInfo*> orderedPrimitiveList;
  BVHBuildNode* pRoot = recursiveBuild(pModel->getPrimitiveInfoListP(), 0, pModel->getPrimitiveInfoListP()->size(), &this->totalNodes, orderedPrimitiveList);

  int offset = 0;
  this->pLinearNodeBuffer = (LinearBVHNode*)malloc(sizeof(LinearBVHNode) * this->totalNodes);
  flattenBVHTree(this->pLinearNodeBuffer, pRoot, &offset);
  recursiveFree(pRoot);

  int currentVertex = 0;
  this->pOrderedVertexBuffer = (float*)malloc(sizeof(float) * orderedPrimitiveList.size() * 6 * 3);
  for (uint64_t x = 0; x < orderedPrimitiveList.size(); x++) {
    memcpy(this->pOrderedVertexBuffer + currentVertex + 0, orderedPrimitiveList[x]->vertexA, sizeof(float) * 3);
    memcpy(this->pOrderedVertexBuffer + currentVertex + 3, orderedPrimitiveList[x]->vertexB, sizeof(float) * 3);
    memcpy(this->pOrderedVertexBuffer + currentVertex + 6, orderedPrimitiveList[x]->vertexC, sizeof(float) * 3);
    memcpy(this->pOrderedVertexBuffer + currentVertex + 9, orderedPrimitiveList[x]->normalA, sizeof(float) * 3);
    memcpy(this->pOrderedVertexBuffer + currentVertex + 12, orderedPrimitiveList[x]->normalB, sizeof(float) * 3);
    memcpy(this->pOrderedVertexBuffer + currentVertex + 15, orderedPrimitiveList[x]->normalC, sizeof(float) * 3);
    currentVertex += 18;
  }
}

AccelerationStructure::~AccelerationStructure() {

}

BVHBuildNode* AccelerationStructure::recursiveBuild(std::vector<PrimitiveInfo>* pPrimitiveInfoList, int start, int end, int* pTotalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList) {
  std::vector<PrimitiveInfo>& primitiveInfoList = *pPrimitiveInfoList;

  BVHBuildNode* node = new BVHBuildNode();
  (*pTotalNodes) += 1;

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

    node->pLeftChild = NULL;
    node->pRightChild = NULL;

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

      node->pLeftChild = NULL;
      node->pRightChild = NULL;

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

      node->pLeftChild = recursiveBuild(pPrimitiveInfoList, start, mid, pTotalNodes, orderedPrimitiveList);
      node->pRightChild = recursiveBuild(pPrimitiveInfoList, mid, end, pTotalNodes, orderedPrimitiveList);
    }
  }

  return node;
}

void AccelerationStructure::recursiveFree(BVHBuildNode* pNode) {
  if (pNode == NULL) {
    return;
  }

  recursiveFree(pNode->pLeftChild);
  recursiveFree(pNode->pRightChild);

  delete pNode;
}

int AccelerationStructure::flattenBVHTree(LinearBVHNode* pLinearBVHNodes, BVHBuildNode* pNode, int* pOffset) {
  LinearBVHNode* pLinearNode = &pLinearBVHNodes[*pOffset];
  memcpy(pLinearNode->boundsMin, pNode->boundsMin, sizeof(float) * 3);
  memcpy(pLinearNode->boundsMax, pNode->boundsMax, sizeof(float) * 3);

  int currentOffset = (*pOffset)++;

  if (pNode->primitiveCount > 0) {
    pLinearNode->primitivesOffset = pNode->firstPrimitiveOffset;
    pLinearNode->primitiveCount = pNode->primitiveCount;
  }
  else {
    pLinearNode->axis = pNode->splitAxis;
    pLinearNode->primitiveCount = 0;
    flattenBVHTree(pLinearBVHNodes, pNode->pLeftChild, pOffset);
    pLinearNode->secondChildOffset = flattenBVHTree(pLinearBVHNodes, pNode->pRightChild, pOffset);
  }

  return currentOffset;
}

uint64_t AccelerationStructure::getNodeBufferSize() {
  return sizeof(LinearBVHNode) * this->totalNodes;
}

void* AccelerationStructure::getNodeBuffer() {
  return this->pLinearNodeBuffer;
}

uint64_t AccelerationStructure::getOrderedVertexBufferSize() {
  return sizeof(float) * this->totalPrimitives * 6 * 3;
}

void* AccelerationStructure::getOrderedVertexBuffer() {
  return this->pOrderedVertexBuffer;
}