#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define SAMPLE_COUNT 25
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

float4 uniformSampleHemisphere(float2 uv) {
  float z = uv.x;
  float r = sqrt(fmax(0.0f, 1.0f - z * z));
  float phi = 2.0 * M_PI * uv.y;

  return (float4)(r * cos(phi), z, r * sin(phi), 0);
}

float4 alignHemisphereWithCoordinateSystem(float4 hemisphere, float4 up) {
  float4 right = normalize(cross(up, (float4)(0.0072f, 1.0f, 0.0034f, 0)));
  float4 forward = cross(right, up);

  return hemisphere.x * right + hemisphere.y * up + hemisphere.z * forward;
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

float4 extractDataFromBarycentrics(float3 barycentrics, float* a, float* b, float* c) {
  float3 dataA = (float3)(a[0], a[1], a[2]);
  float3 dataB = (float3)(b[0], b[1], b[2]);
  float3 dataC = (float3)(c[0], c[1], c[2]);
  return (float4)(dataA * barycentrics.x + dataB * barycentrics.y + dataC * barycentrics.z, 1.0);
}

float3 shade(__global LinearBVHNode* linearNodes, 
             __global Primitive* primitives, 
             __global Material* materials,
             __global LightContainer* lightContainers,
             Ray cameraRay,
             float2 filmPosition,
             uint sampleIndex) {

  float3 directColor = (float3)(0, 0, 0);
  float3 indirectColor = (float3)(0, 0, 0);

  RayPayload rayPayload = {0, 0, FLT_MAX, 0, 0};
  Ray ray = cameraRay;
  intersect(&rayPayload, ray, linearNodes, primitives);

  bool hitLight = false;
  for (int x = 0; x < lightContainers[0].count; x++) {
    if (rayPayload.primitiveIndex == lightContainers[0].primitives[x]) {
      hitLight = true;
    }
  }

  if (hitLight) {
    directColor = (float3)(1, 1, 1);
  }
  else {
    if (rayPayload.hitType == 1) {
      __global Primitive* primitive = &primitives[rayPayload.primitiveIndex];
      __global Material* material = &materials[primitive->materialIndex];

      float3 barycentrics = (float3)(1.0 - rayPayload.u - rayPayload.v, rayPayload.u, rayPayload.v);
      float4 position = extractDataFromBarycentrics(barycentrics, primitive->positionA, primitive->positionB, primitive->positionC);
      float4 normal = extractDataFromBarycentrics(barycentrics, primitive->normalA, primitive->normalB, primitive->normalC);

      int lightRandomIndex = random(filmPosition, sampleIndex) * lightContainers[0].count;
      Primitive* lightPrimitive = &primitives[lightContainers[0].primitives[lightRandomIndex]];

      float2 uv = (float2)(random(filmPosition, sampleIndex + 1), random(filmPosition, sampleIndex + 2));
      if (uv.x + uv.y > 1.0f) {
        uv.x = 1.0f - uv.x;
        uv.y = 1.0f - uv.y;
      }
      float3 lightBarycentric = (float3)(1.0 - uv.x - uv.y, uv.x, uv.y);
      float4 lightPosition = extractDataFromBarycentrics(lightBarycentric, lightPrimitive->positionA, lightPrimitive->positionB, lightPrimitive->positionC);

      float4 positionToLight = normalize(lightPosition - position);

      RayPayload shadowRayPayload = {0, 0, distance(position, lightPosition) - SHADOW_RAY_EPSILON, 0, 0};
      Ray shadowRay = {
        .origin = position,
        .direction = positionToLight,
      };
      intersectIgnorePrimitiveIndex(&shadowRayPayload, shadowRay, linearNodes, primitives, rayPayload.primitiveIndex);

      if (shadowRayPayload.hitType == 0) {
        directColor = (float3)(material->diffuse[0], material->diffuse[1], material->diffuse[2]) * dot(positionToLight, normal);
      }

      float4 hemisphere = uniformSampleHemisphere((float2)(random(filmPosition, sampleIndex + 3), random(filmPosition, sampleIndex + 4)));
      float4 alignedHemisphere = alignHemisphereWithCoordinateSystem(hemisphere, normal);

      Ray extensionRay = {position, alignedHemisphere};
      float4 previousNormal = normal;
      int previousPrimitive = rayPayload.primitiveIndex;

      bool rayActive = true;
      int maxRayDepth = 16;
      for (int rayDepth = 0; rayDepth < maxRayDepth && rayActive; rayDepth++) {
        RayPayload extensionRayPayload = {0, 0, FLT_MAX, 0, 0};
        intersectIgnorePrimitiveIndex(&extensionRayPayload, extensionRay, linearNodes, primitives, previousPrimitive);

        bool extensionHitLight = false;
        for (int x = 0; x < lightContainers[0].count; x++) {
          if (extensionRayPayload.primitiveIndex == lightContainers[0].primitives[x]) {
            extensionHitLight = true;
          }
        }

        if (extensionHitLight) {
          indirectColor += (float3)(1.0 / (rayDepth + 1)) * (float3)(1, 1, 1) * dot(previousNormal, extensionRay.direction);
        }
        else {
          if (extensionRayPayload.hitType == 1) {
            __global Primitive* extensionPrimitive = &primitives[extensionRayPayload.primitiveIndex];
            __global Material* extensionMaterial = &materials[extensionPrimitive->materialIndex];

            float3 extensionBarycentrics = (float3)(1.0 - extensionRayPayload.u - extensionRayPayload.v, extensionRayPayload.u, extensionRayPayload.v);
            float4 extensionPosition = extractDataFromBarycentrics(extensionBarycentrics, extensionPrimitive->positionA, extensionPrimitive->positionB, extensionPrimitive->positionC);
            float4 extensionNormal = extractDataFromBarycentrics(extensionBarycentrics, extensionPrimitive->normalA, extensionPrimitive->normalB, extensionPrimitive->normalC);

            int extensionLightRandomIndex = random(filmPosition, sampleIndex + rayDepth + 5) * lightContainers[0].count;
            Primitive* extensionLightPrimitive = &primitives[lightContainers[0].primitives[extensionLightRandomIndex]];

            float2 extensionUV = (float2)(random(filmPosition, sampleIndex + rayDepth + 6), random(filmPosition, sampleIndex + rayDepth + 7));
            if (extensionUV.x + extensionUV.y > 1.0f) {
              extensionUV.x = 1.0f - extensionUV.x;
              extensionUV.y = 1.0f - extensionUV.y;
            }
            float3 extensionLightBarycentric = (float3)(1.0 - extensionUV.x - extensionUV.y, extensionUV.x, extensionUV.y);
            float4 extensionLightPosition = extractDataFromBarycentrics(extensionLightBarycentric, extensionLightPrimitive->positionA, extensionLightPrimitive->positionB, extensionLightPrimitive->positionC);

            float4 extensionPositionToLight = normalize(extensionLightPosition - extensionPosition);

            RayPayload extensionShadowRayPayload = {0, 0, distance(extensionPosition, extensionLightPosition) - SHADOW_RAY_EPSILON, 0, 0};
            Ray extensionShadowRay = {
              .origin = extensionPosition,
              .direction = extensionPositionToLight,
            };
            intersectIgnorePrimitiveIndex(&extensionShadowRayPayload, extensionShadowRay, linearNodes, primitives, extensionRayPayload.primitiveIndex);
          
            if (extensionShadowRayPayload.hitType == 0) {
              indirectColor += (float3)(1.0 / (rayDepth + 1)) * (float3)(extensionMaterial->diffuse[0], extensionMaterial->diffuse[1], extensionMaterial->diffuse[2]) * dot(extensionPositionToLight, extensionNormal);
            
              hemisphere = uniformSampleHemisphere((float2)(random(filmPosition, sampleIndex + rayDepth + 8), random(filmPosition, sampleIndex + rayDepth + 9)));
              alignedHemisphere = alignHemisphereWithCoordinateSystem(hemisphere, extensionNormal);

              extensionRay.origin = extensionPosition;
              extensionRay.direction = alignedHemisphere;
              previousNormal = extensionNormal;
              previousPrimitive = extensionRayPayload.primitiveIndex;
            }
            else {
              rayActive = false;
            }
          }
          else {
            rayActive = false;
          }
        }
      }
    }
  }

  return directColor + indirectColor;
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

  float3 color = shade(linearNodes, primitives, materials, lightContainers, ray, filmPosition.xy, camera->frameCount * 32 + 0);
  for (int x = 1; x < SAMPLE_COUNT; x++) {
    float attenuationFactor = ((float)(SAMPLE_COUNT - x)) / SAMPLE_COUNT;
    float3 colorNew = shade(linearNodes, primitives, materials, lightContainers, ray, filmPosition.xy, camera->frameCount * 32 + x);
    float3 colorAtten = ((1.0f - attenuationFactor) * color) + (attenuationFactor * colorNew);

    color = colorAtten;
  }

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

  float3 color = shade(linearNodes, primitives, materials, lightContainers, ray, filmPosition.xy, camera->frameCount * 32 + 0);
  for (int x = 1; x < SAMPLE_COUNT; x++) {
    float attenuationFactor = ((float)(SAMPLE_COUNT - x)) / SAMPLE_COUNT;
    float3 colorNew = shade(linearNodes, primitives, materials, lightContainers, ray, filmPosition.xy, camera->frameCount * 32 + x);
    float3 colorAtten = ((1.0f - attenuationFactor) * color) + (attenuationFactor * colorNew);

    color = colorAtten;
  }

  output[blockID + 0] = color.x;
  output[blockID + 1] = color.y;
  output[blockID + 2] = color.z;
}