#include <stdint.h>
// #include <optix.h>
// #include <optix_device.h>
// #include <optix_host.h>
// #include <optix_stubs.h>

extern "C" void createAccelerationStructure(void* vertexBuffer, uint32_t vertexCount, void* indexBuffer, uint32_t indexCount) {
  // cudaFree(0);
  // optixInit();

  // OptixDeviceContextOptions options = {};
  // CUcontext cuCtx = 0;
  // OptixDeviceContext context = NULL;
  // optixDeviceContextCreate(cuCtx, &options, &context);

  // OptixAccelBuildOptions accelBuildOptions = {
  //   .buildFlags = OPTIX_BUILD_FLAG_NONE,
  //   .operation = OPTIX_BUILD_OPERATION_BUILD,
  //   .motionOptions = {
  //     .numKeys = 0,
  //     .flags = 0,
  //     .timeBegin = 0,
  //     .timeEnd = 0
  //   }
  // };

  // const uint32_t buildInputTriangleArrayFlags[1] = { OPTIX_GEOMETRY_FLAG_NONE };

  // CUdeviceptr vertexBufferDevice = 0;
  // cudaMalloc((void**)(&vertexBufferDevice), vertexCount * sizeof(float));
  // cudaMemcpy((void*)vertexBufferDevice, vertexBuffer, vertexCount * sizeof(float), cudaMemcpyHostToDevice);

  // CUdeviceptr indexBufferDevice = 0;
  // cudaMalloc((void**)(&indexBufferDevice), indexCount * sizeof(uint32_t));
  // cudaMemcpy((void*)indexBufferDevice, indexBuffer, indexCount * sizeof(uint32_t), cudaMemcpyHostToDevice);

  // OptixBuildInputTriangleArray buildInputTriangleArray = {
  //   .vertexBuffers = &vertexBufferDevice,
  //   .numVertices = vertexCount,
  //   .vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3,
  //   .vertexStrideInBytes = sizeof(float3),
  //   .indexBuffer = indexBufferDevice,
  //   .numIndexTriplets = indexCount / 3,
  //   .indexFormat = OPTIX_INDICES_FORMAT_UNSIGNED_INT3,
  //   .indexStrideInBytes = sizeof(int3),
  //   .preTransform = 0,
  //   .flags = buildInputTriangleArrayFlags,
  //   .numSbtRecords = 1,
  //   .sbtIndexOffsetBuffer = 0,
  //   .sbtIndexOffsetSizeInBytes = sizeof(int),
  //   .sbtIndexOffsetStrideInBytes = sizeof(int),
  //   .primitiveIndexOffset = 0,
  //   .transformFormat = OPTIX_TRANSFORM_FORMAT_NONE
  // };

  // OptixBuildInput buildInput = {
  //   .type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES,
  //   .triangleArray = buildInputTriangleArray
  // };

  // OptixAccelBufferSizes accelBufferSizes = {};
  // optixAccelComputeMemoryUsage(context, &accelBuildOptions, &buildInput, 1, &accelBufferSizes);

  // CUdeviceptr accelerationStructureBufferDevice = 0;
  // CUdeviceptr scratchBufferDevice = 0;
  // cudaMalloc((void**)&accelerationStructureBufferDevice, accelBufferSizes.outputSizeInBytes);
  // cudaMalloc((void**)&scratchBufferDevice, accelBufferSizes.tempSizeInBytes);

  // OptixTraversableHandle outputHandle = 0;
  // optixAccelBuild(
  //   context, 
  //   0, 
  //   &accelBuildOptions, 
  //   &buildInput, 
  //   1, 
  //   scratchBufferDevice, 
  //   accelBufferSizes.tempSizeInBytes, 
  //   accelerationStructureBufferDevice,
  //   accelBufferSizes.outputSizeInBytes, 
  //   &outputHandle, 
  //   NULL, 
  //   0
  // );
}