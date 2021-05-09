#pragma once

#include <string>
#include <fstream>

class Resource {
public:
  static std::string findResource(std::string resourcePath);
};