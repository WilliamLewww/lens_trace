#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif

#include "model.h"

Model::Model(std::string fileName) {
  chdir("res");

  this->fileName = fileName;
  this->success = tinyobj::LoadObj(&this->attrib, &this->shapes, &this->materials, &this->warning, &this->error, fileName.c_str());

  chdir("..");

  this->checkError();
}

Model::~Model() {

}

std::string Model::getFileName() {
  return this->fileName;
}

bool Model::checkError() {
  if (!this->warning.empty()) {
    printf("%s\n", this->warning.c_str());
  }

  if (!this->error.empty()) {
    printf("%s\n", this->error.c_str());
  }

  return this->success;
}

tinyobj::attrib_t Model::getAttrib() {
  return this->attrib;
}

std::vector<tinyobj::shape_t> Model::getShapes() {
  return this->shapes;
}