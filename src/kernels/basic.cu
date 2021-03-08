#include <stdio.h>

__global__
void linearKernel(float* pOutputBuffer, int width, int height) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  int idy = blockIdx.y * blockDim.y + threadIdx.y;
  int id = idy * width + idx;

  if (idx >= width || idy >= height) {
    return;
  }

  pOutputBuffer[id] = id;
}

extern "C" void linearKernelWrapper(float* pOutputBuffer, int width, int height) {
  dim3 block(32, 32);
  dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);

  float* pOutputBufferDevice;
  cudaMalloc(&pOutputBufferDevice, sizeof(float) * width * height);

  linearKernel<<<grid, block>>>(pOutputBufferDevice, width, height);
  cudaDeviceSynchronize();

  cudaError_t error = cudaGetLastError();
  if (error != cudaSuccess) {
    printf("%s\n", cudaGetErrorString(error));
  }

  cudaMemcpy(pOutputBuffer, pOutputBufferDevice, sizeof(float) * width * height, cudaMemcpyDeviceToHost);
  cudaFree(pOutputBufferDevice);
}

extern "C" void tileKernelWrapper() {

}