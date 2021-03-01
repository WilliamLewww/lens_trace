#pragma once

#include "structures.h"
#include "model.h"

#include <algorithm>
#include <string.h>

struct BVHBuildNode {
  float boundsMin[3];
  float boundsMax[3];
  BVHBuildNode* leftChild;
  BVHBuildNode* rightChild;

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

class AccelerationStructure {
private:
  int totalNodes;
  LinearBVHNode* linearNodeBuffer;

  int totalPrimitives;
  float* orderedVertexBuffer;

  BVHBuildNode* recursiveBuild(std::vector<PrimitiveInfo>* pPrimitiveInfoList, int start, int end, int* totalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList);
  void recursiveFree(BVHBuildNode* node);

  int flattenBVHTree(LinearBVHNode* linearBVHNodes, BVHBuildNode* node, int* offset);
public:
  AccelerationStructure(AccelerationStructureProperties accelerationStructureProperties);
  ~AccelerationStructure();

  uint64_t getNodeBufferSize();
  void* getNodeBuffer();

  uint64_t getOrderedVertexBufferSize();
  void* getOrderedVertexBuffer();
};