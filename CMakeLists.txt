cmake_minimum_required (VERSION 2.8.12)
project(LENS_TRACE LANGUAGES CXX CUDA)

find_package(CUDA REQUIRED)
find_package(OpenCL REQUIRED)

file(GLOB SOURCES "src/*.cpp" "kernels/*.cu")

add_library(lensTrace
  ${SOURCES}
)

include_directories(include)

add_executable(LensTrace src/main.cpp)
target_link_libraries(LensTrace OpenCL::OpenCL)
target_link_libraries(LensTrace lensTrace)

add_custom_target(copy_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
)

add_custom_target(copy_kernels
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/kernels/runtime_compiled ${CMAKE_CURRENT_BINARY_DIR}/kernels
)

add_dependencies(LensTrace copy_resources)
add_dependencies(LensTrace copy_kernels)