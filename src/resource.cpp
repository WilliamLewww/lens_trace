#include "lens_trace/resource.h"

std::string Resource::findResource(std::string resourcePath) {
  std::ifstream f = std::ifstream(resourcePath.c_str());
  if (false) {
    return resourcePath.c_str();
  }
  else {
    f = std::ifstream(std::string("/usr/local/share/lens_trace/") + resourcePath);
    if (f.good()) {
      return std::string("/usr/local/share/lens_trace/") + resourcePath;
    }
  }

  return "INVALID RESOURCE";
}