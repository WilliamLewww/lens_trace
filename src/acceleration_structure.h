#pragma once

#include "model.h"

#include <algorithm>
#include <string.h>

struct PrimitiveInfo {
  int index;
  float vertexA[3];
  float vertexB[3];
  float vertexC[3];

  float boundsMin[3];
  float boundsMax[3];
  float centroid[3];
};

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
  BVHBuildNode* recursiveBuild(std::vector<PrimitiveInfo>& primitiveInfoList, int start, int end, int* totalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList);
  void recursiveFree(BVHBuildNode* node);

  int flattenBVHTree(LinearBVHNode* linearBVHNodes, BVHBuildNode* node, int* offset);
public:
  AccelerationStructure(Model* model);
  ~AccelerationStructure();
};