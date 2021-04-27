cmake_minimum_required (VERSION 2.8.12)
project(LENS_TRACE LANGUAGES CXX)

find_package(CUDA)
find_package(OpenCL)

if(NOT CUDA_FOUND AND NOT OpenCL_FOUND)
  message(FATAL_ERROR "Could not find CUDA or OpenCL. Exiting")
endif()

file(GLOB SOURCES "lens_trace/src/*.cpp" "lens_trace/src/kernels/*.cu")

add_library(lensTrace
  ${SOURCES}
)

include_directories(LensTrace lens_trace/include)
add_executable(LensTrace lens_trace/src/main.cpp)

if(CUDA_FOUND)
  enable_language(CUDA)
  target_compile_definitions(lensTrace PRIVATE -DCUDA_ENABLED)
endif()

if(OpenCL_FOUND)
  target_compile_definitions(lensTrace PRIVATE -DOPENCL_ENABLED)
  target_link_libraries(LensTrace ${OpenCL_LIBRARIES})
endif()

target_link_libraries(LensTrace lensTrace)

add_custom_target(copy_resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
)

add_dependencies(LensTrace copy_resources)