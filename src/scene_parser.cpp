#include "lens_trace/scene_parser.h"

SceneParser::SceneParser(std::string filename) {
  std::ifstream ifs(filename);
  nlohmann::json jf = nlohmann::json::parse(ifs);
  
  if (jf["renderer"] != nullptr) {
    if (jf["renderer"]["render_platform"] != nullptr) {
      if (jf["renderer"]["render_platform"] == "RENDER_PLATFORM_OPENCL") {
        this->rendererParsed.renderPlatform = RENDER_PLATFORM_OPENCL;
        if (jf["renderer"]["kernel_file_path"] != nullptr) {
          this->rendererParsed.kernelFilePath = jf["renderer"]["kernel_file_path"];
        }
      }
      if (jf["renderer"]["render_platform"] == "RENDER_PLATFORM_CUDA") {
        this->rendererParsed.renderPlatform = RENDER_PLATFORM_CUDA;
        if (jf["renderer"]["kernel_file_path"] != nullptr) {
          this->rendererParsed.kernelFilePath = jf["renderer"]["kernel_file_path"];
        }
      }
    }
    if (jf["renderer"]["kernel_mode"] != nullptr) {
      if (jf["renderer"]["kernel_mode"] == "KERNEL_MODE_LINEAR")
        this->rendererParsed.kernelMode = KERNEL_MODE_LINEAR;
      if (jf["renderer"]["kernel_mode"] == "KERNEL_MODE_TILE")
        this->rendererParsed.kernelMode = KERNEL_MODE_TILE;
    }
    if (jf["renderer"]["thread_organization_mode"] != nullptr) {
      if (jf["renderer"]["thread_organization_mode"] == "THREAD_ORGANIZATION_MODE_MAX_FIT")
        this->rendererParsed.threadOrganizationMode = THREAD_ORGANIZATION_MODE_MAX_FIT;
      if (jf["renderer"]["thread_organization_mode"] == "THREAD_ORGANIZATION_MODE_CUSTOM") {
        this->rendererParsed.threadOrganizationMode = THREAD_ORGANIZATION_MODE_CUSTOM;
        if (jf["renderer"]["render_platform"] == "RENDER_PLATFORM_OPENCL") {
          this->rendererParsed.workBlockSize[0] = jf["renderer"]["work_block_size"][0];
          this->rendererParsed.workBlockSize[1] = jf["renderer"]["work_block_size"][1];
          this->rendererParsed.threadGroupSize[0] = jf["renderer"]["thread_group_size"][0];
          this->rendererParsed.threadGroupSize[1] = jf["renderer"]["thread_group_size"][1];
        }
        if (jf["renderer"]["render_platform"] == "RENDER_PLATFORM_CUDA") {
          this->rendererParsed.blockSize[0] = jf["renderer"]["block_size"][0];
          this->rendererParsed.blockSize[1] = jf["renderer"]["block_size"][1];
        }
      }
    }
    if (jf["renderer"]["image_dimensions"] != nullptr) {
      this->rendererParsed.imageDimensions[0] = jf["renderer"]["image_dimensions"][0];
      this->rendererParsed.imageDimensions[1] = jf["renderer"]["image_dimensions"][1];
      this->rendererParsed.imageDimensions[2] = jf["renderer"]["image_dimensions"][2];
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

uint64_t SceneParser::getOutputBufferSize() {
  return sizeof(float) * this->rendererParsed.imageDimensions[0] * this->rendererParsed.imageDimensions[1] * this->rendererParsed.imageDimensions[2];
}

RenderPlatform SceneParser::getRenderPlatform() {
  return this->rendererParsed.renderPlatform;
}

void* SceneParser::createOutputBuffer() {
  return malloc(this->getOutputBufferSize());
}

Camera* SceneParser::createCamera() {
  return new Camera(this->cameraParsed.position[0], this->cameraParsed.position[1], this->cameraParsed.position[2], this->cameraParsed.yaw);
}

Model* SceneParser::createModel() {
  return new Model(this->worldParsed.models[0].filePath);
}

AccelerationStructureExplicit* SceneParser::createAccelerationStructure(Model* model) {
  AccelerationStructureExplicitProperties accelerationStructureExplicitProperties = {
    .sType = STRUCTURE_TYPE_ACCELERATION_STRUCTURE_PROPERTIES,
    .pNext = NULL,
    .accelerationStructureExplicitType = ACCELERATION_STRUCTURE_TYPE_BVH,
    .pModel = model,
  };
  return new AccelerationStructureExplicit(accelerationStructureExplicitProperties);
}

RenderPropertiesOpenCL SceneParser::getRenderPropertiesOpenCL(void* outputBuffer, AccelerationStructureExplicit* accelerationStructureExplicit, Model* model, Camera* camera) {
  RenderPropertiesOpenCL renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_OPENCL,
    .pNext = NULL,
    .kernelFilePath = this->rendererParsed.kernelFilePath,
    .kernelMode = this->rendererParsed.kernelMode,
    .threadOrganizationMode = this->rendererParsed.threadOrganizationMode,
    .threadOrganization = {},
    .imageDimensions = {this->rendererParsed.imageDimensions[0], this->rendererParsed.imageDimensions[1], this->rendererParsed.imageDimensions[2]},
    .pOutputBuffer = outputBuffer,
    .outputBufferSize = this->getOutputBufferSize(),
    .pAccelerationStructureExplicit = accelerationStructureExplicit,
    .pModel = model,
    .pCamera = camera
  };
  if (this->rendererParsed.threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
    renderProperties.threadOrganization = {
      .sType = STRUCTURE_TYPE_THREAD_ORGANIZATION_OPENCL,
      .pNext = NULL,
      .workBlockSize = {this->rendererParsed.workBlockSize[0], this->rendererParsed.workBlockSize[1]},
      .threadGroupSize = {this->rendererParsed.threadGroupSize[0], this->rendererParsed.threadGroupSize[1]},
    };
  }
  return renderProperties;
}

RenderPropertiesCUDA SceneParser::getRenderPropertiesCUDA(void* outputBuffer, AccelerationStructureExplicit* accelerationStructureExplicit, Model* model, Camera* camera) {
  RenderPropertiesCUDA renderProperties = {
    .sType = STRUCTURE_TYPE_RENDER_PROPERTIES_CUDA,
    .pNext = NULL,
    .kernelFilePath = this->rendererParsed.kernelFilePath,
    .kernelMode = this->rendererParsed.kernelMode,
    .threadOrganizationMode = this->rendererParsed.threadOrganizationMode,
    .threadOrganization = {},
    .imageDimensions = {this->rendererParsed.imageDimensions[0], this->rendererParsed.imageDimensions[1], this->rendererParsed.imageDimensions[2]},
    .pOutputBuffer = outputBuffer,
    .outputBufferSize = this->getOutputBufferSize(),
    .pAccelerationStructureExplicit = accelerationStructureExplicit,
    .pModel = model,
    .pCamera = camera
  };
  if (this->rendererParsed.threadOrganizationMode == THREAD_ORGANIZATION_MODE_CUSTOM) {
    renderProperties.threadOrganization = {
      .sType = STRUCTURE_TYPE_THREAD_ORGANIZATION_CUDA,
      .pNext = NULL,
      .blockSize = {this->rendererParsed.blockSize[0], this->rendererParsed.blockSize[1]}
    };
  }
  return renderProperties;
}

BufferToImageProperties SceneParser::getBufferToImageProperties(void* outputBuffer) {
  BufferToImageProperties bufferToImageProperties = {
    .sType = STRUCTURE_TYPE_BUFFER_TO_IMAGE_PROPERTIES,
    .pNext = NULL,
    .pBuffer = outputBuffer,
    .bufferSize = this->getOutputBufferSize(),
    .imageDimensions = {this->rendererParsed.imageDimensions[0], this->rendererParsed.imageDimensions[1], this->rendererParsed.imageDimensions[2]},
    .imageType = IMAGE_TYPE_JPEG,
    .filename = this->outputParsed.filePath.c_str()
  };
  return bufferToImageProperties;
}