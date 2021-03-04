#pragma once

#include "structures.h"
#include "model.h"

#include <algorithm>
#include <string.h>

struct BVHBuildNode {
  float boundsMin[3];
  float boundsMax[3];
  BVHBuildNode* pLeftChild;
  BVHBuildNode* pRightChild;

  int splitAxis;
  int firstPrimitiveOffset;
  int primitiveCount;
};

struct LinearBVHNode {
  float boundsMin[3];
  float boundsMax[3];

  union {
    int primitivesOffset;
    int secondChildOffset;
  };

  uint16_t primitiveCount;
  uint8_t axis;
  uint8_t pad[1];
};

struct Primitive {
  float vertexA[3];
  float vertexB[3];
  float vertexC[3];
  float normalA[3];
  float normalB[3];
  float normalC[3];
  int materialIndex;
};

class AccelerationStructure {
private:
  int totalNodes;
  LinearBVHNode* pLinearNodeBuffer;

  int totalPrimitives;
  Primitive* pOrderedPrimitiveBuffer;

  BVHBuildNode* recursiveBuild(std::vector<PrimitiveInfo>* pPrimitiveInfoList, int start, int end, int* pTotalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList);
  void recursiveFree(BVHBuildNode* pNode);

  int flattenBVHTree(LinearBVHNode* pLinearBVHNodes, BVHBuildNode* pNode, int* pOffset);
public:
  AccelerationStructure(AccelerationStructureProperties accelerationStructureProperties);
  ~AccelerationStructure();

  uint64_t getNodeBufferSize();
  void* getNodeBuffer();

  uint64_t getOrderedPrimitiveBufferSize();
  void* getOrderedPrimitiveBuffer();
};