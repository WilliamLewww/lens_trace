cmake_minimum_required (VERSION 2.8.12)
project(LENS_TRACE LANGUAGES CXX CUDA)

find_package(CUDA REQUIRED)
find_package(OpenCL REQUIRED)

file(GLOB SOURCES "src/*.cpp" "kernels/*.cu")

add_library(program
  ${SOURCES}
)

include_directories(include)

add_executable(LensTrace src/main.cpp)
target_link_libraries(LensTrace OpenCL::OpenCL)
target_link_libraries(LensTrace program)

file(COPY res DESTINATION .)
file(GLOB RUNTIME_COMPILED_KERNELS
  "kernels/runtime_compiled/*"
)
file(COPY ${RUNTIME_COMPILED_KERNELS} DESTINATION kernels)