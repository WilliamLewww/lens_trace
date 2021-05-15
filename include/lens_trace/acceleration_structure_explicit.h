#pragma once

#include "lens_trace/structures.h"
#include "lens_trace/model.h"

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
  uint32_t index;
  float positionA[3];
  float positionB[3];
  float positionC[3];
  float normalA[3];
  float normalB[3];
  float normalC[3];
  int materialIndex;
};

class AccelerationStructureExplicit {
private:
  int totalNodes;
  LinearBVHNode* pLinearNodeBuffer;

  int totalPrimitives;
  Primitive* pOrderedPrimitiveBuffer;

  BVHBuildNode* recursiveBuild(std::vector<PrimitiveInfo>* pPrimitiveInfoList, int start, int end, int* pTotalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList);
  void recursiveFree(BVHBuildNode* pNode);

  int flattenBVHTree(LinearBVHNode* pLinearBVHNodes, BVHBuildNode* pNode, int* pOffset);
public:
  AccelerationStructureExplicit(AccelerationStructureExplicitProperties accelerationStructureExplicitProperties);
  ~AccelerationStructureExplicit();

  uint64_t getNodeBufferSize();
  void* getNodeBuffer();

  uint64_t getOrderedPrimitiveBufferSize();
  void* getOrderedPrimitiveBuffer();
};