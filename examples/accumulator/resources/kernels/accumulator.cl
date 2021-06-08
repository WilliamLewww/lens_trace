#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define TRIANGLE_INTERSECTION_EPSILON 0.0001
#define SHADOW_RAY_EPSILON 0.01

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

inline float random(float2 uv, float seed) {
  float a = sin(fmod(dot(uv, (float2)(12.9898, 78.233)) + 1113.1 * seed, M_PI)) * 43758.5453;
  return a - floor(a);
}

__global const float* getBounds(int dirIsNeg, __global const float* boundsMin, __global const float* boundsMax) {
  return (dirIsNeg == 0) ? boundsMin : boundsMax;
}

bool intersectTriangle(RayPayload* rayPayload, Ray ray, Primitive primitive) {
  float4 positionA = (float4)(primitive.positionA[0], primitive.positionA[1], primitive.positionA[2], 1);
  float4 positionB = (float4)(primitive.positionB[0], primitive.positionB[1], primitive.positionB[2], 1);
  float4 positionC = (float4)(primitive.positionC[0], primitive.positionC[1], primitive.positionC[2], 1);

  float4 v0v1 = positionB - positionA;
  float4 v0v2 = positionC - positionA;
  float4 pvec = cross(ray.direction, v0v2);
  float det = dot(v0v1, pvec);

  if (fabs(det) < TRIANGLE_INTERSECTION_EPSILON) {
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
    __global LinearBVHNode* node = &nodes[currentNodeIndex];

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
             __global LightContainer* lightContainers,
             Ray cameraRay,
             float2 filmPosition,
             uint sampleIndex) {

  float3 outputColor = (float3)(0, 0, 0);

  RayPayload rayPayload = {0, 0, FLT_MAX, 0, 0};
  Ray ray = cameraRay;
  intersect(&rayPayload, ray, linearNodes, primitives);

  for (int x = 0; x < lightContainers[0].count; x++) {
    if (rayPayload.primitiveIndex == lightContainers[0].primitives[x]) {
      return (float3)(1, 1, 1);
    }
  }

  if (rayPayload.hitType == 1) {
    __global Primitive* primitive = &primitives[rayPayload.primitiveIndex];
    __global Material* material = &materials[primitive->materialIndex];

    float3 barycentrics = (float3)(1.0 - rayPayload.u - rayPayload.v, rayPayload.u, rayPayload.v);
    float3 positionA = (float3)(primitive->positionA[0], primitive->positionA[1], primitive->positionA[2]);
    float3 positionB = (float3)(primitive->positionB[0], primitive->positionB[1], primitive->positionB[2]);
    float3 positionC = (float3)(primitive->positionC[0], primitive->positionC[1], primitive->positionC[2]);
    float4 position = (float4)(positionA * barycentrics.x + positionB * barycentrics.y + positionC * barycentrics.z, 1.0);
    float3 normalA = (float3)(primitive->normalA[0], primitive->normalA[1], primitive->normalA[2]);
    float3 normalB = (float3)(primitive->normalB[0], primitive->normalB[1], primitive->normalB[2]);
    float3 normalC = (float3)(primitive->normalC[0], primitive->normalC[1], primitive->normalC[2]);
    float4 normal = (float4)(normalA * barycentrics.x + normalB * barycentrics.y + normalC * barycentrics.z, 0.0);

    int lightRandomIndex = random(filmPosition, sampleIndex) * lightContainers[0].count;
    Primitive* lightPrimitive = &primitives[lightContainers[0].primitives[lightRandomIndex]];

    float2 uv = (float2)(random(filmPosition, sampleIndex + 1), random(filmPosition, sampleIndex + 2));
    if (uv.x + uv.y > 1.0f) {
      uv.x = 1.0f - uv.x;
      uv.y = 1.0f - uv.y;
    }
    float3 lightBarycentric = (float3)(1.0 - uv.x - uv.y, uv.x, uv.y);
    float3 lightPositionA = (float3)(lightPrimitive->positionA[0], lightPrimitive->positionA[1], lightPrimitive->positionA[2]);
    float3 lightPositionB = (float3)(lightPrimitive->positionB[0], lightPrimitive->positionB[1], lightPrimitive->positionB[2]);
    float3 lightPositionC = (float3)(lightPrimitive->positionC[0], lightPrimitive->positionC[1], lightPrimitive->positionC[2]);
    float4 lightPosition = (float4)(lightPositionA * lightBarycentric.x + lightPositionB * lightBarycentric.y + lightPositionC * lightBarycentric.z, 1.0);

    float4 positionToLight = normalize(lightPosition - position);

    RayPayload shadowRayPayload = {0, 0, distance(position, lightPosition) - SHADOW_RAY_EPSILON, 0, 0};
    Ray shadowRay = {
      .origin = position,
      .direction = positionToLight,
    };
    intersectIgnorePrimitiveIndex(&shadowRayPayload, shadowRay, linearNodes, primitives, rayPayload.primitiveIndex);

    if (shadowRayPayload.hitType == 0) {
      outputColor = (float3)(material->diffuse[0], material->diffuse[1], material->diffuse[2]) * dot(positionToLight, normal);
    }
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

  float3 color = shade(linearNodes, primitives, materials, lightContainers, ray, filmPosition.xy, camera->frameCount);

  output[blockID + 0] = clamp(color.x, 0.0f, 1.0f);
  output[blockID + 1] = clamp(color.y, 0.0f, 1.0f);
  output[blockID + 2] = clamp(color.z, 0.0f, 1.0f);
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

  float3 color = shade(linearNodes, primitives, materials, lightContainers, ray, filmPosition.xy, camera->frameCount);

  output[blockID + 0] = color.x;
  output[blockID + 1] = color.y;
  output[blockID + 2] = color.z;
}