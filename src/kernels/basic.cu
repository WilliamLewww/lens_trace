#include <stdio.h>
#include <stdint.h>

struct LinearBVHNode {
  float boundsMin[3];
  float boundsMax[3];

  union {
    int primitivesOffset;
    int secondChildOffset;
  };

  ushort primitiveCount;
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
};

struct Camera {
  float position[3];
  float yaw;
};

__global__
void linearKernel(LinearBVHNode* linearNodes, 
                  Primitive* primitives,
                  Material* materials, 
                  Camera* camera,
                  float* output, 
                  int width, 
                  int height, 
                  int depth) {

  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  int idy = blockIdx.y * blockDim.y + threadIdx.y;
  int id = (idy * width + idx) * depth;

  if (idx >= width || idy >= height) {
    return;
  }

  output[id] = id;
}

extern "C" void linearKernelWrapper(void* linearNodeBuffer,
                                    uint64_t linearNodeBufferSize,
                                    void* primitiveBuffer,
                                    uint64_t primitiveBufferSize,
                                    void* materialBuffer,
                                    uint64_t materialBufferSize,
                                    void* cameraBuffer,
                                    uint64_t cameraBufferSize,
                                    void* outputBuffer, 
                                    int width, 
                                    int height, 
                                    int depth) {

  dim3 block(32, 32);
  dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);

  void* linearNodeBufferDevice;
  cudaMalloc(&linearNodeBufferDevice, linearNodeBufferSize);
  cudaMemcpy(linearNodeBufferDevice, linearNodeBuffer, linearNodeBufferSize, cudaMemcpyHostToDevice);

  void* primitiveBufferDevice;
  cudaMalloc(&primitiveBufferDevice, primitiveBufferSize);
  cudaMemcpy(primitiveBufferDevice, primitiveBuffer, primitiveBufferSize, cudaMemcpyHostToDevice);

  void* materialBufferDevice;
  cudaMalloc(&materialBufferDevice, materialBufferSize);
  cudaMemcpy(materialBufferDevice, materialBuffer, materialBufferSize, cudaMemcpyHostToDevice);

  void* cameraBufferDevice;
  cudaMalloc(&cameraBufferDevice, cameraBufferSize);
  cudaMemcpy(cameraBufferDevice, cameraBuffer, cameraBufferSize, cudaMemcpyHostToDevice);

  void* outputBufferDevice;
  cudaMalloc(&outputBufferDevice, sizeof(float) * width * height * depth);

  linearKernel<<<grid, block>>>(
    (LinearBVHNode*)linearNodeBufferDevice, 
    (Primitive*)primitiveBufferDevice, 
    (Material*)materialBufferDevice, 
    (Camera*)cameraBufferDevice, 
    (float*)outputBufferDevice, 
    width, 
    height, 
    depth
  );
  cudaDeviceSynchronize();

  cudaError_t error = cudaGetLastError();
  if (error != cudaSuccess) {
    printf("%s\n", cudaGetErrorString(error));
  }

  cudaMemcpy(outputBuffer, outputBufferDevice, sizeof(float) * width * height * depth, cudaMemcpyDeviceToHost);
  cudaFree(outputBufferDevice);
}

extern "C" void tileKernelWrapper() {

}