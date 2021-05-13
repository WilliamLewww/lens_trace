cmake_minimum_required (VERSION 2.8.12)
project(example_custom_kernel LANGUAGES CXX)

find_path(LENS_TRACE_INCLUDE_DIRECTORY
	NAMES lens_trace
	HINTS /usr/local/include/lens_trace
)

find_library(LENS_TRACE_LIBRARY
	NAMES lenstrace
	HINTS /usr/local/lib/lens_trace
)

find_library(LENS_TRACE_CUDA_LIBRARY
	NAMES lenstrace-cuda
	HINTS /usr/local/lib/lens_trace
)

find_library(LENS_TRACE_OPENCL_LIBRARY
	NAMES lenstrace-opencl
	HINTS /usr/local/lib/lens_trace
)

add_executable(example_custom_kernel src/main.cpp)
include_directories(example_custom_kernel ${LENS_TRACE_INCLUDE_DIRECTORY})
target_link_libraries(example_custom_kernel ${LENS_TRACE_LIBRARY} ${LENS_TRACE_CUDA_LIBRARY} ${LENS_TRACE_OPENCL_LIBRARY})

add_custom_target(copy_resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
)

add_dependencies(example_custom_kernel copy_resources)