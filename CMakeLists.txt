cmake_minimum_required (VERSION 2.8.12)
project(LENS_TRACE LANGUAGES CXX CUDA)

find_package(CUDA REQUIRED)
find_package(OpenCL REQUIRED)

file(GLOB SOURCES "lens_trace/src/*.cpp" "lens_trace/kernels/*.cu")

add_library(lensTrace
  ${SOURCES}
)

include_directories(lens_trace/include)

add_executable(LensTrace lens_trace/src/main.cpp)
target_link_libraries(LensTrace OpenCL::OpenCL)
target_link_libraries(LensTrace lensTrace)

add_custom_target(copy_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
)

add_custom_target(copy_kernels
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/runtime_kernels ${CMAKE_CURRENT_BINARY_DIR}/kernels
)

add_dependencies(LensTrace copy_resources)
add_dependencies(LensTrace copy_kernels)