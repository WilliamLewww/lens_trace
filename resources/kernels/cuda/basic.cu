#define FLT_MAX 10000000

enum KernelMode {
  KERNEL_MODE_LINEAR,
  KERNEL_MODE_TILE
};

struct LinearBVHNode {
  float boundsMin[3];
  float boundsMax[3];

  union {
    int primitivesOffset;
    int secondChildOffset;
  };

  unsigned short primitiveCount;
  unsigned char axis;
  unsigned char pad[1];
};

struct Primitive {
  float positionA[3];
  float positionB[3];
  float positionC[3];
  float normalA[3];
  float normalB[3];
  float normalC[3];
  int materialIndex;
};

struct Material {
  float diffuse[3];
  float ior;
  float dissolve;
  float emission[3];
};

struct LightContainer {
  unsigned int count;
  unsigned int primitives[64];
};

struct Camera {
  float position[3];
  float yaw;
  float pitch;
  float roll;
  unsigned int frameCount;
};

struct Ray {
  float4 origin;
  float4 direction;
};

struct RayPayload {
  int primitiveIndex;
  int hitType;
  float t;
  float u;
  float v;
};

inline __device__ float3 operator*(float3 a, float b) { return make_float3(a.x * b, a.y * b, a.z * b); }
inline __device__ float4 operator*(float a, float4 b) { return make_float4(a * b.x, a * b.y, a * b.z, a * b.w); }
inline __device__ float3 operator+(float3 a, float3 b) { return make_float3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline __device__ float4 operator+(float4 a, float4 b) { return make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline __device__ float4 operator-(float4 a, float4 b) { return make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline __device__ float4 cross(float4 a, float4 b) { return make_float4(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x, a.w); }
inline __device__ float dot(float4 a, float4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

inline __device__ 
float4 reflect(float4 incidentDirection, float4 normal) {
  float cosI = -dot(normal, incidentDirection);
  return incidentDirection + 2.0f * cosI * normal;
}

inline __device__
float4 refract(float4 incidentDirection, float4 normal, float firstIOR, float secondIOR) {
  float n = firstIOR / secondIOR;
  float cosI = -dot(normal, incidentDirection);
  float sinT2 = n * n * (1.0 - cosI * cosI);
  float cosT = sqrt(1.0 - sinT2);
  return n * incidentDirection + (n * cosI - cosT) * normal;
}

__device__
const float* getBounds(int dirIsNeg, const float* boundsMin, const float* boundsMax) {
  return (dirIsNeg == 0) ? boundsMin : boundsMax;
}

__device__
bool intersectTriangle(RayPayload* rayPayload, Ray ray, Primitive primitive) {
  const float EPSILON = 0.0000001;
  float4 positionA = make_float4(primitive.positionA[0], primitive.positionA[1], primitive.positionA[2], 1);
  float4 positionB = make_float4(primitive.positionB[0], primitive.positionB[1], primitive.positionB[2], 1);
  float4 positionC = make_float4(primitive.positionC[0], primitive.positionC[1], primitive.positionC[2], 1);

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

__device__
bool intersectBounds(Ray ray, float4 invDir, int dirIsNeg[3], const float* boundsMin, const float* boundsMax) {
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

__device__
void intersect(RayPayload* rayPayload, Ray ray, LinearBVHNode* nodes, Primitive* primitives) {
  float4 invDir = make_float4(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z, 0);
  int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

  int toVisitOffset = 0, currentNodeIndex = 0;
  int nodesToVisit[64];
  while (true) {
    const LinearBVHNode* node = &nodes[currentNodeIndex];

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

__device__
void intersectIgnorePrimitiveIndex(RayPayload* rayPayload, 
                                   Ray ray, 
                                   LinearBVHNode* nodes, 
                                   Primitive* primitives, 
                                   int ignorePrimitiveIndex) {

  float4 invDir = make_float4(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z, 0);
  int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

  int toVisitOffset = 0, currentNodeIndex = 0;
  int nodesToVisit[64];
  while (true) {
    const LinearBVHNode* node = &nodes[currentNodeIndex];

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

__device__
void traceRayThroughLens(LinearBVHNode* linearNodes, 
                         Primitive* primitives, 
                         Material* materials,
                         RayPayload* rayPayload,
                         Ray* ray) {

  float3 outputColor = make_float3(0, 0, 0);

  Primitive* primitive = &primitives[rayPayload->primitiveIndex];
  Material* material = &materials[primitive->materialIndex];

  float3 barycentrics = make_float3(1.0 - rayPayload->u - rayPayload->v, rayPayload->u, rayPayload->v);
  float3 positionA = make_float3(primitive->positionA[0], primitive->positionA[1], primitive->positionA[2]);
  float3 positionB = make_float3(primitive->positionB[0], primitive->positionB[1], primitive->positionB[2]);
  float3 positionC = make_float3(primitive->positionC[0], primitive->positionC[1], primitive->positionC[2]);
  float3 position = positionA * barycentrics.x + positionB * barycentrics.y + positionC * barycentrics.z;
  float3 normalA = make_float3(primitive->normalA[0], primitive->normalA[1], primitive->normalA[2]);
  float3 normalB = make_float3(primitive->normalB[0], primitive->normalB[1], primitive->normalB[2]);
  float3 normalC = make_float3(primitive->normalC[0], primitive->normalC[1], primitive->normalC[2]);
  float3 normal = normalA * barycentrics.x + normalB * barycentrics.y + normalC * barycentrics.z;

  float4 transmissionDirection = refract(ray->direction, make_float4(normal.x, normal.y, normal.z, 0.0), 1.0, material->ior);

  RayPayload rayPayload2 = {0, 0, FLT_MAX, 0, 0};
  Ray ray2 = {make_float4(position.x, position.y, position.z, 1.0), transmissionDirection};
  intersectIgnorePrimitiveIndex(&rayPayload2, ray2, linearNodes, primitives, rayPayload->primitiveIndex);

  primitive = &primitives[rayPayload2.primitiveIndex];
  material = &materials[primitive->materialIndex];

  barycentrics = make_float3(1.0 - rayPayload2.u - rayPayload2.v, rayPayload2.u, rayPayload2.v);
  positionA = make_float3(primitive->positionA[0], primitive->positionA[1], primitive->positionA[2]);
  positionB = make_float3(primitive->positionB[0], primitive->positionB[1], primitive->positionB[2]);
  positionC = make_float3(primitive->positionC[0], primitive->positionC[1], primitive->positionC[2]);
  position = positionA * barycentrics.x + positionB * barycentrics.y + positionC * barycentrics.z;
  normalA = make_float3(primitive->normalA[0], primitive->normalA[1], primitive->normalA[2]);
  normalB = make_float3(primitive->normalB[0], primitive->normalB[1], primitive->normalB[2]);
  normalC = make_float3(primitive->normalC[0], primitive->normalC[1], primitive->normalC[2]);
  normal = normalA * barycentrics.x + normalB * barycentrics.y + normalC * barycentrics.z;

  transmissionDirection = refract(transmissionDirection, make_float4(-normal.x, -normal.y, -normal.z, 0.0), material->ior, 1.0);

  rayPayload->primitiveIndex = 0;
  rayPayload->hitType = 0;
  rayPayload->t = FLT_MAX;
  rayPayload->u = 0;
  rayPayload->v = 0;

  ray->origin = make_float4(position.x, position.y, position.z, 1.0);
  ray->direction = transmissionDirection;

  intersectIgnorePrimitiveIndex(rayPayload, *ray, linearNodes, primitives, rayPayload2.primitiveIndex);
}

__device__
float3 shade(LinearBVHNode* linearNodes, 
             Primitive* primitives, 
             Material* materials,
             Ray cameraRay) {

  float3 outputColor = make_float3(0, 0, 0);

  RayPayload rayPayload = {0, 0, FLT_MAX, 0, 0};
  Ray ray = cameraRay;
  intersect(&rayPayload, ray, linearNodes, primitives);

  if (rayPayload.hitType == 1) {
    Primitive* primitive = &primitives[rayPayload.primitiveIndex];
    Material* material = &materials[primitive->materialIndex];

    if (material->dissolve < 1.0) {
      traceRayThroughLens(linearNodes, primitives, materials, &rayPayload, &ray);

      if (rayPayload.hitType == 1) {
        primitive = &primitives[rayPayload.primitiveIndex];
        material = &materials[primitive->materialIndex];
      }
    }

    outputColor = make_float3(material->diffuse[0], material->diffuse[1], material->diffuse[2]);
  }

  return outputColor;
}

extern "C" __global__
void linearKernel(LinearBVHNode* linearNodes, 
                  Primitive* primitives,
                  Material* materials,
                  LightContainer* lightContainers,
                  Camera* camera,
                  float* output, 
                  int width, 
                  int height, 
                  int depth) {

  int idy = blockIdx.y * blockDim.y + threadIdx.y;
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  int id = (idy * width + idx) * depth;

  if (idx >= width || idy >= height) {
    return;
  }

  float4 cameraPosition = make_float4(camera->position[0], camera->position[1], camera->position[2], 1);
  float4 filmPosition = make_float4(((float)idx / width) - 0.5f, ((float)idy / height) - 0.5f, 0, 1);
  float4 aperaturePosition = make_float4(0.0, 0.0, 5, 1);

  Ray ray = {cameraPosition + filmPosition, aperaturePosition - filmPosition};
  float newX = (cos(camera->yaw) * ray.direction.x) + (sin(camera->yaw) * ray.direction.z);
  float newZ = (-sin(camera->yaw) * ray.direction.x) + (cos(camera->yaw) * ray.direction.z);
  ray.direction.x = newX;
  ray.direction.z = newZ;
  float3 outputColor = shade(linearNodes, primitives, materials, ray);

  output[id + 0] = outputColor.x;
  output[id + 1] = outputColor.y;
  output[id + 2] = outputColor.z;
}

extern "C" __global__
void tileKernel(LinearBVHNode* linearNodes, 
                Primitive* primitives,
                Material* materials,
                LightContainer* lightContainers,
                Camera* camera,
                float* output, 
                int width, 
                int height, 
                int depth) {

  int currentBlock = blockIdx.y * gridDim.x + blockIdx.x;
  int idy = ((currentBlock / gridDim.x) * blockDim.y) + threadIdx.y;
  int idx = ((currentBlock % gridDim.x) * blockDim.x) + threadIdx.x;
  int id = (idy * width + idx) * depth;

  if (idx >= width || idy >= height) {
    return;
  }

  float4 cameraPosition = make_float4(camera->position[0], camera->position[1], camera->position[2], 1);
  float4 filmPosition = make_float4(((float)idx / width) - 0.5f, ((float)idy / height) - 0.5f, 0, 1);
  float4 aperaturePosition = make_float4(0.0, 0.0, 5, 1);

  Ray ray = {cameraPosition + filmPosition, aperaturePosition - filmPosition};
  float newX = (cos(camera->yaw) * ray.direction.x) + (sin(camera->yaw) * ray.direction.z);
  float newZ = (-sin(camera->yaw) * ray.direction.x) + (cos(camera->yaw) * ray.direction.z);
  ray.direction.x = newX;
  ray.direction.z = newZ;
  float3 outputColor = shade(linearNodes, primitives, materials, ray);

  output[id + 0] = outputColor.x;
  output[id + 1] = outputColor.y;
  output[id + 2] = outputColor.z;
}