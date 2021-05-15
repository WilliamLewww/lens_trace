#include "lens_trace/acceleration_structure_explicit.h"

AccelerationStructureExplicit::AccelerationStructureExplicit(AccelerationStructureExplicitProperties accelerationStructureExplicitProperties) {
  Model* pModel = (Model*)accelerationStructureExplicitProperties.pModel;
  Material* pMaterial = (Material*)pModel->getMaterialBuffer();

  this->totalPrimitives = pModel->getPrimitiveInfoListP()->size();
  this->totalNodes = 0;
  std::vector<PrimitiveInfo*> orderedPrimitiveList;
  BVHBuildNode* pRoot = recursiveBuild(pModel->getPrimitiveInfoListP(), 0, pModel->getPrimitiveInfoListP()->size(), &this->totalNodes, orderedPrimitiveList);

  int offset = 0;
  this->pLinearNodeBuffer = (LinearBVHNode*)malloc(sizeof(LinearBVHNode) * this->totalNodes);
  flattenBVHTree(this->pLinearNodeBuffer, pRoot, &offset);
  recursiveFree(pRoot);

  this->lightContainer = {
    .count = 0,
    .primitives = {}
  };

  this->pOrderedPrimitiveBuffer = (Primitive*)malloc(sizeof(Primitive) * this->totalPrimitives);
  for (uint64_t x = 0; x < orderedPrimitiveList.size(); x++) {
    memcpy(this->pOrderedPrimitiveBuffer[x].positionA, orderedPrimitiveList[x]->positionA, sizeof(float) * 3);
    memcpy(this->pOrderedPrimitiveBuffer[x].positionB, orderedPrimitiveList[x]->positionB, sizeof(float) * 3);
    memcpy(this->pOrderedPrimitiveBuffer[x].positionC, orderedPrimitiveList[x]->positionC, sizeof(float) * 3);
    memcpy(this->pOrderedPrimitiveBuffer[x].normalA, orderedPrimitiveList[x]->normalA, sizeof(float) * 3);
    memcpy(this->pOrderedPrimitiveBuffer[x].normalB, orderedPrimitiveList[x]->normalB, sizeof(float) * 3);
    memcpy(this->pOrderedPrimitiveBuffer[x].normalC, orderedPrimitiveList[x]->normalC, sizeof(float) * 3);
    memcpy(&this->pOrderedPrimitiveBuffer[x].materialIndex, &orderedPrimitiveList[x]->materialIndex, sizeof(int));
    
    Material material = pMaterial[orderedPrimitiveList[x]->materialIndex];
    if (material.emission[0] > 0 ||
        material.emission[1] > 0 ||
        material.emission[2] > 0) {

      lightContainer.primitives[lightContainer.count] = x;
      lightContainer.count += 1;
    }
  }
}

AccelerationStructureExplicit::~AccelerationStructureExplicit() {
  free(this->pLinearNodeBuffer);
}

BVHBuildNode* AccelerationStructureExplicit::recursiveBuild(std::vector<PrimitiveInfo>* pPrimitiveInfoList, int start, int end, int* pTotalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList) {
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

void AccelerationStructureExplicit::recursiveFree(BVHBuildNode* pNode) {
  if (pNode == NULL) {
    return;
  }

  recursiveFree(pNode->pLeftChild);
  recursiveFree(pNode->pRightChild);

  delete pNode;
}

int AccelerationStructureExplicit::flattenBVHTree(LinearBVHNode* pLinearBVHNodes, BVHBuildNode* pNode, int* pOffset) {
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

uint64_t AccelerationStructureExplicit::getNodeBufferSize() {
  return sizeof(LinearBVHNode) * this->totalNodes;
}

void* AccelerationStructureExplicit::getNodeBuffer() {
  return this->pLinearNodeBuffer;
}

uint64_t AccelerationStructureExplicit::getOrderedPrimitiveBufferSize() {
  return sizeof(Primitive) * this->totalPrimitives;
}

void* AccelerationStructureExplicit::getOrderedPrimitiveBuffer() {
  return this->pOrderedPrimitiveBuffer;
}

uint64_t AccelerationStructureExplicit::getLightContainerBufferSize() {
  return sizeof(LightContainer);
}

void* AccelerationStructureExplicit::getLightContainerBuffer() {
  return &this->lightContainer;
}