#include "lens_trace/scene_parser.h"

SceneParser::SceneParser(std::string filename) {
  std::ifstream ifs(filename);
  nlohmann::json jf = nlohmann::json::parse(ifs);
  
  if (jf["engine"] != nullptr) {
    if (jf["engine"]["render_platform"] != nullptr) {
      if (jf["engine"]["render_platform"] == "RENDER_PLATFORM_OPENCL")
        this->engineParsed.renderPlatform = RENDER_PLATFORM_OPENCL;
      if (jf["engine"]["render_platform"] == "RENDER_PLATFORM_CUDA")
        this->engineParsed.renderPlatform = RENDER_PLATFORM_CUDA;
    }
    if (jf["engine"]["kernel_mode"] != nullptr) {
      if (jf["engine"]["kernel_mode"] == "KERNEL_MODE_LINEAR")
        this->engineParsed.kernelMode = KERNEL_MODE_LINEAR;
      if (jf["engine"]["kernel_mode"] == "KERNEL_MODE_TILE")
        this->engineParsed.kernelMode = KERNEL_MODE_TILE;
    }
    if (jf["engine"]["thread_organization_mode"] != nullptr) {
      if (jf["engine"]["thread_organization_mode"] == "THREAD_ORGANIZATION_MODE_MAX_FIT")
        this->engineParsed.threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT;
      if (jf["engine"]["thread_organization_mode"] == "THREAD_ORGANIZATION_MODE_CUSTOM") {
        this->engineParsed.threadOrganizationMode = THREAD_ORGANIZATION_MODE_CUSTOM;
        if (jf["engine"]["render_platform"] == "RENDER_PLATFORM_OPENCL") {
          this->engineParsed.workBlockSize[0] = jf["engine"]["work_block_size"][0];
          this->engineParsed.workBlockSize[1] = jf["engine"]["work_block_size"][1];
          this->engineParsed.threadGroupSize[0] = jf["engine"]["thread_group_size"][0];
          this->engineParsed.threadGroupSize[1] = jf["engine"]["thread_group_size"][1];
        }
        if (jf["engine"]["render_platform"] == "RENDER_PLATFORM_CUDA") {
          this->engineParsed.blockSize[0] = jf["engine"]["block_size"][0];
          this->engineParsed.blockSize[1] = jf["engine"]["block_size"][1];
        }
      }
    }
    if (jf["engine"]["image_dimensions"] != nullptr) {
      this->engineParsed.imageDimensions[0] = jf["engine"]["image_dimensions"][0];
      this->engineParsed.imageDimensions[1] = jf["engine"]["image_dimensions"][1];
      this->engineParsed.imageDimensions[2] = jf["engine"]["image_dimensions"][2];
    }
  }

  if (jf["camera"] != nullptr) {
    if (jf["camera"]["position"] != nullptr) {
      this->cameraParsed.position[0] = jf["camera"]["position"][0];
      this->cameraParsed.position[1] = jf["camera"]["position"][1];
      this->cameraParsed.position[2] = jf["camera"]["position"][2];
    }
    if (jf["camera"]["pitch"] != nullptr) {
      this->cameraParsed.pitch = jf["camera"]["pitch"];
    }
    if (jf["camera"]["yaw"] != nullptr) {
      this->cameraParsed.yaw = jf["camera"]["yaw"];
    }
    if (jf["camera"]["roll"] != nullptr) {
      this->cameraParsed.roll = jf["camera"]["roll"];
    }
  }

  if (jf["world"] != nullptr) {
    for (nlohmann::json::iterator it = jf["world"].begin(); it != jf["world"].end(); it++) {
      ModelParsed model = {
        .filePath = jf["world"][it.key()]["file_path"]
      };
      worldParsed.models.push_back(model);
    }
  }

  if (jf["output"] != nullptr) {
    if (jf["output"]["file_path"] != nullptr) {
      this->outputParsed.filePath = jf["output"]["file_path"];
    }
  }
}

SceneParser::~SceneParser() {

}

void SceneParser::renderScene() {
  uint64_t outputBufferSize = sizeof(float) * this->engineParsed.imageDimensions[0] * this->engineParsed.imageDimensions[1] * this->engineParsed.imageDimensions[2];
  void* pOutputBuffer = malloc(outputBufferSize);

  Camera* pCamera = new Camera(this->cameraParsed.position[0], this->cameraParsed.position[1], this->cameraParsed.position[2], this->cameraParsed.yaw);
  Model* pModel = new Model(this->worldParsed.models[0].filePath);

  AccelerationStructureExplicitProperties accelerationStructureExplicitProperties = {
    .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
    .pNext = NULL,
    .accelerationStructureExplicitType = ACCELERATION_STRUCTURE_TYPE_BVH,
    .pModel = pModel,
  };
  AccelerationStructureExplicit* pAccelerationStructureExplicit = new AccelerationStructureExplicit(accelerationStructureExplicitProperties);

  Engine* pEngine = new Engine(this->engineParsed.renderPlatform);

  if (this->engineParsed.renderPlatform == RENDER_PLATFORM_OPENCL) {
    RenderPropertiesOpenCL renderProperties = {
      .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
      .pNext = NULL,
      .kernelMode = this->engineParsed.kernelMode,
      .threadOrganizationMode = this->engineParsed.threadOrganizationMode,
      .pThreadOrganization = NULL,
      .imageDimensions = {this->engineParsed.imageDimensions[0], this->engineParsed.imageDimensions[1], this->engineParsed.imageDimensions[2]},
      .pOutputBuffer = pOutputBuffer,
      .outputBufferSize = outputBufferSize,
      .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
      .pModel = pModel,
      .pCamera = pCamera
    };

    if (this->engineParsed.threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
      ThreadOrganizationOpenCL threadOrganization = {
        .sType = STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL,
        .pNext = NULL,
        .workBlockSize = {this->engineParsed.workBlockSize[0], this->engineParsed.workBlockSize[1]},
        .threadGroupSize = {this->engineParsed.threadGroupSize[0], this->engineParsed.threadGroupSize[1]},
      };

      renderProperties.pThreadOrganization = &threadOrganization;
    }

    pEngine->render(&renderProperties);
  }

  if (this->engineParsed.renderPlatform == RENDER_PLATFORM_CUDA) {
    RenderPropertiesCUDA renderProperties = {
      .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
      .pNext = NULL,
      .kernelMode = this->engineParsed.kernelMode,
      .threadOrganizationMode = this->engineParsed.threadOrganizationMode,
      .pThreadOrganization = NULL,
      .imageDimensions = {this->engineParsed.imageDimensions[0], this->engineParsed.imageDimensions[1], this->engineParsed.imageDimensions[2]},
      .pOutputBuffer = pOutputBuffer,
      .outputBufferSize = outputBufferSize,
      .pAccelerationStructureExplicit = pAccelerationStructureExplicit,
      .pModel = pModel,
      .pCamera = pCamera
    };

    if (this->engineParsed.threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
      ThreadOrganizationCUDA threadOrganization = {
        .sType = STRUCTURE_TYPE_THREAD_ORGANIZATION_CUDA,
        .pNext = NULL,
        .blockSize = {this->engineParsed.blockSize[0], this->engineParsed.blockSize[1]}
      };

      renderProperties.pThreadOrganization = &threadOrganization;
    }

    pEngine->render(&renderProperties);
  }

  BufferToImageProperties bufferToImageProperties = {
    .sType = STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES,
    .pNext = NULL,
    .pBuffer = pOutputBuffer,
    .bufferSize = outputBufferSize,
    .imageDimensions = {this->engineParsed.imageDimensions[0], this->engineParsed.imageDimensions[1], this->engineParsed.imageDimensions[2]},
    .imageType = IMAGE_TYPE_JPEG,
    .filename = this->outputParsed.filePath.c_str()
  };
  pEngine->writeBufferToImage(bufferToImageProperties);

  delete pCamera;
  delete pModel;
  delete pAccelerationStructureExplicit;
  delete pEngine;
  free(pOutputBuffer);
}