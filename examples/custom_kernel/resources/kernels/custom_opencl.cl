#pragma OPENCL EXTENSION cl_khr_fp64 : enable

typedef struct CameraTag {
  float position[3];
  float yaw;
  float pitch;
  float roll;
  uint frameCount;
} Camera;

typedef struct LinearBVHNodeTag {
  float boundsMin[3];
  float boundsMax[3];

  union {
    int primitivesOffset;
    int secondChildOffset;
  };

  ushort primitiveCount;
  uchar axis;
  uchar pad[1];
} LinearBVHNode;

typedef struct PrimitiveTag {
  float positionA[3];
  float positionB[3];
  float positionC[3];
  float normalA[3];
  float normalB[3];
  float normalC[3];
  int materialIndex;
} Primitive;

typedef struct MaterialTag {
  float diffuse[3];
  float ior;
  float dissolve;
  float emission[3];
} Material;

typedef struct LightContainerTag {
  uint count;
  uint primitives[64];
} LightContainer;

typedef struct RayTag {
  float4 origin;
  float4 direction;
} Ray;

typedef struct RayPayloadTag {
  int primitiveIndex;
  int hitType;
  float t;
  float u;
  float v;
} RayPayload;

inline float4 reflect(float4 incidentDirection, float4 normal) {
  float cosI = -dot(normal, incidentDirection);
  return incidentDirection + 2.0f * cosI * normal;
}

inline float4 refract(float4 incidentDirection, float4 normal, float firstIOR, float secondIOR) {
  float n = firstIOR / secondIOR;
  float cosI = -dot(normal, incidentDirection);
  float sinT2 = n * n * (1.0 - cosI * cosI);
  float cosT = sqrt(1.0 - sinT2);
  return n * incidentDirection + (n * cosI - cosT) * normal;
}

__global const float* getBounds(int dirIsNeg, __global const float* boundsMin, __global const float* boundsMax) {
  return (dirIsNeg == 0) ? boundsMin : boundsMax;
}

bool intersectTriangle(RayPayload* rayPayload, Ray ray, Primitive primitive) {
  const float EPSILON = 0.0000001;
  float4 positionA = (float4)(primitive.positionA[0], primitive.positionA[1], primitive.positionA[2], 1);
  float4 positionB = (float4)(primitive.positionB[0], primitive.positionB[1], primitive.positionB[2], 1);
  float4 positionC = (float4)(primitive.positionC[0], primitive.positionC[1], primitive.positionC[2], 1);

  float4 v0v1 = positionB - positionA;
  float4 v0v2 = positionC - positionA;
  float4 pvec = cross(ray.direction, v0v2);
  float det = dot(v0v1, pvec);

  if (fabs(det) < EPSILON) {
    return false;
  }

  float invDet = 1 / det;

  float4 tvec = ray.origin - positionA;
  float u = dot(tvec,pvec) * invDet;
  if (u < 0 || u > 1) {
    return false;
  }

  float4 qvec = cross(tvec, v0v1);
  float v = dot(ray.direction, qvec) * invDet;
  if (v < 0 || u + v > 1) {
    return false;
  }

  float t = dot(v0v2, qvec) * invDet;

  if (t < rayPayload->t) {
    rayPayload->t = t;
    rayPayload->u = u;
    rayPayload->v = v;

    return true;
  }

  return false;
}

bool intersectBounds(Ray ray, float4 invDir, int dirIsNeg[3], __global const float* boundsMin, __global const float* boundsMax) {
  float tMin = (getBounds(dirIsNeg[0], boundsMin, boundsMax)[0] - ray.origin.x) * invDir.x;
  float tMax = (getBounds(1 - dirIsNeg[0], boundsMin, boundsMax)[0] - ray.origin.x) * invDir.x;
  float tyMin = (getBounds(dirIsNeg[1], boundsMin, boundsMax)[1] - ray.origin.y) * invDir.y;
  float tyMax = (getBounds(1 - dirIsNeg[1], boundsMin, boundsMax)[1] - ray.origin.y) * invDir.y;

  if (tMin > tyMax || tyMin > tMax) return false;
  if (tyMin > tMin) tMin = tyMin;
  if (tyMax < tMax) tMax = tyMax;

  float tzMin = (getBounds(dirIsNeg[2], boundsMin, boundsMax)[2] - ray.origin.z) * invDir.z;
  float tzMax = (getBounds(1 - dirIsNeg[2], boundsMin, boundsMax)[2] - ray.origin.z) * invDir.z;

  if (tMin > tzMax || tzMin > tMax) return false;
  if (tzMin > tMin) tMin = tzMin;
  if (tzMax < tMax) tMax = tzMax;
  return (tMax > 0);
}

void intersect(RayPayload* rayPayload, Ray ray, __global LinearBVHNode* nodes, __global Primitive* primitives) {
  float4 invDir = (float4)(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z, 0);
  int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

  int toVisitOffset = 0, currentNodeIndex = 0;
  int nodesToVisit[64];
  while (true) {
    __global const LinearBVHNode* node = &nodes[currentNodeIndex];

    if (intersectBounds(ray, invDir, dirIsNeg, node->boundsMin, node->boundsMax)) {
      if (node->primitiveCount > 0) {
        for (int i = 0; i < node->primitiveCount; i++) {
          if (intersectTriangle(rayPayload, ray, primitives[node->primitivesOffset])) {
            rayPayload->primitiveIndex = node->primitivesOffset;
            rayPayload->hitType = 1;
          }
        }
        if (toVisitOffset == 0) {
          break;
        }
        currentNodeIndex = nodesToVisit[--toVisitOffset];
      }
      else {
        if (dirIsNeg[node->axis]) {
          nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
          currentNodeIndex = node->secondChildOffset;
        } else {
          nodesToVisit[toVisitOffset++] = node->secondChildOffset;
          currentNodeIndex = currentNodeIndex + 1;
        }
      }
    }
    else {
      if (toVisitOffset == 0) {
        break;
      }
      currentNodeIndex = nodesToVisit[--toVisitOffset];
    }
  }
}

void intersectIgnorePrimitiveIndex(RayPayload* rayPayload, 
                                   Ray ray, 
                                   __global LinearBVHNode* nodes, 
                                   __global Primitive* primitives, 
                                   int ignorePrimitiveIndex) {

  float4 invDir = (float4)(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z, 0);
  int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

  int toVisitOffset = 0, currentNodeIndex = 0;
  int nodesToVisit[64];
  while (true) {
    __global const LinearBVHNode* node = &nodes[currentNodeIndex];

    if (intersectBounds(ray, invDir, dirIsNeg, node->boundsMin, node->boundsMax)) {
      if (node->primitiveCount > 0) {
        for (int i = 0; i < node->primitiveCount; i++) {
          if (node->primitivesOffset != ignorePrimitiveIndex && intersectTriangle(rayPayload, ray, primitives[node->primitivesOffset])) {
            rayPayload->primitiveIndex = node->primitivesOffset;
            rayPayload->hitType = 1;
          }
        }
        if (toVisitOffset == 0) {
          break;
        }
        currentNodeIndex = nodesToVisit[--toVisitOffset];
      }
      else {
        if (dirIsNeg[node->axis]) {
          nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
          currentNodeIndex = node->secondChildOffset;
        } else {
          nodesToVisit[toVisitOffset++] = node->secondChildOffset;
          currentNodeIndex = currentNodeIndex + 1;
        }
      }
    }
    else {
      if (toVisitOffset == 0) {
        break;
      }
      currentNodeIndex = nodesToVisit[--toVisitOffset];
    }
  }
}

float3 shade(__global LinearBVHNode* linearNodes, 
             __global Primitive* primitives, 
             __global Material* materials,
             Ray cameraRay) {

  float3 outputColor = (float3)(0, 0, 0);

  RayPayload rayPayload = {0, 0, FLT_MAX, 0, 0};
  Ray ray = cameraRay;
  intersect(&rayPayload, ray, linearNodes, primitives);

  if (rayPayload.hitType == 1) {
    __global Primitive* primitive = &primitives[rayPayload.primitiveIndex];
    __global Material* material = &materials[primitive->materialIndex];

    outputColor = (float3)(rayPayload.u, rayPayload.v, 1.0 - rayPayload.u - rayPayload.v);
  }

  return outputColor;
}

__kernel 
void linearKernel(__global LinearBVHNode* linearNodes, 
                  __global Primitive* primitives, 
                  __global Material* materials,
                  __global LightContainer* lightContainers,
                  __global Camera* camera,
                  __global float* output,
                  uint currentBlock, 
                  uint width, 
                  uint height, 
                  uint depth) {

  int blockIDY = ((currentBlock / (width / get_global_size(0))) * get_global_size(1)) + get_global_id(1);
  int blockIDX = ((currentBlock % (width / get_global_size(0))) * get_global_size(0)) + get_global_id(0);
  int blockID = (blockIDY * width + blockIDX) * depth;

  if (blockIDX >= width || blockIDY >= height) {
    return;
  }

  float4 cameraPosition = (float4)(camera->position[0], camera->position[1], camera->position[2], 1);
  float4 filmPosition = (float4)(((float)blockIDX / width) - 0.5f, ((float)blockIDY / height) - 0.5f, 0, 1);
  float4 aperaturePosition = (float4)(0.0, 0.0, 5, 1);

  Ray ray = {cameraPosition + filmPosition, aperaturePosition - filmPosition};
  float newX = (cos(camera->yaw) * ray.direction.x) + (sin(camera->yaw) * ray.direction.z);
  float newZ = (-sin(camera->yaw) * ray.direction.x) + (cos(camera->yaw) * ray.direction.z);
  ray.direction.x = newX;
  ray.direction.z = newZ;
  float3 outputColor = shade(linearNodes, primitives, materials, ray);

  output[blockID + 0] = outputColor.x;
  output[blockID + 1] = outputColor.y;
  output[blockID + 2] = outputColor.z;
}

__kernel 
void tileKernel(__global LinearBVHNode* linearNodes, 
                __global Primitive* primitives, 
                __global Material* materials,
                __global LightContainer* lightContainers,
                __global Camera* camera,
                __global float* output, 
                uint currentBlock, 
                uint width, 
                uint height, 
                uint depth) {

  int localBlockID = get_group_id(1) * get_num_groups(0) + get_group_id(0);
  int localIDY = ((localBlockID / (get_global_size(0) / get_local_size(0))) * get_local_size(1)) + get_local_id(1);
  int localIDX = ((localBlockID % (get_global_size(0) / get_local_size(0))) * get_local_size(0)) + get_local_id(0);
  int blockIDY = ((currentBlock / (width / get_global_size(0))) * get_global_size(1)) + localIDY;
  int blockIDX = ((currentBlock % (width / get_global_size(0))) * get_global_size(0)) + localIDX;
  int blockID = (blockIDY * width + blockIDX) * depth;

  if (blockIDX >= width || blockIDY >= height) {
    return;
  }

  float4 cameraPosition = (float4)(camera->position[0], camera->position[1], camera->position[2], 1);
  float4 filmPosition = (float4)(((float)blockIDX / width) - 0.5f, ((float)blockIDY / height) - 0.5f, 0, 1);
  float4 aperaturePosition = (float4)(0.0, 0.0, 5, 1);

  Ray ray = {cameraPosition + filmPosition, aperaturePosition - filmPosition};
  float newX = (cos(camera->yaw) * ray.direction.x) + (sin(camera->yaw) * ray.direction.z);
  float newZ = (-sin(camera->yaw) * ray.direction.x) + (cos(camera->yaw) * ray.direction.z);
  ray.direction.x = newX;
  ray.direction.z = newZ;
  float3 outputColor = shade(linearNodes, primitives, materials, ray);

  output[blockID + 0] = outputColor.x;
  output[blockID + 1] = outputColor.y;
  output[blockID + 2] = outputColor.z;
}