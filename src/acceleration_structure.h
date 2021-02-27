#pragma once

#include "model.h"

#include <algorithm>

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

class AccelerationStructure {
private:
  BVHBuildNode* recursiveBuild(std::vector<PrimitiveInfo>& primitiveInfoList, int start, int end, int* totalNodes, std::vector<PrimitiveInfo*>& orderedPrimitiveList);
public:
  AccelerationStructure(Model* model);
  ~AccelerationStructure();
};