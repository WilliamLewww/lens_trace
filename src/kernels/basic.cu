#include <stdio.h>

__global__
void linearKernel(float* pOutputBuffer, int width, int height, int depth) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  int idy = blockIdx.y * blockDim.y + threadIdx.y;
  int id = (idy * width + idx) * depth;

  if (idx >= width || idy >= height) {
    return;
  }

  pOutputBuffer[id] = id;
}

extern "C" void linearKernelWrapper(float* pOutputBuffer, int width, int height, int depth) {
  dim3 block(32, 32);
  dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);

  float* pOutputBufferDevice;
  cudaMalloc(&pOutputBufferDevice, sizeof(float) * width * height * depth);

  linearKernel<<<grid, block>>>(pOutputBufferDevice, width, height, depth);
  cudaDeviceSynchronize();

  cudaError_t error = cudaGetLastError();
  if (error != cudaSuccess) {
    printf("%s\n", cudaGetErrorString(error));
  }

  cudaMemcpy(pOutputBuffer, pOutputBufferDevice, sizeof(float) * width * height * depth, cudaMemcpyDeviceToHost);
  cudaFree(pOutputBufferDevice);
}

extern "C" void tileKernelWrapper() {

}