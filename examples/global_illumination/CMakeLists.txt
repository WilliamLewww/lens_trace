cmake_minimum_required (VERSION 2.8.12)
project(global_illumination LANGUAGES CXX)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

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

add_executable(global_illumination src/main.cpp)
include_directories(global_illumination ${LENS_TRACE_INCLUDE_DIRECTORY})
include_directories(global_illumination ${OPENGL_INCLUDE_DIRS})
include_directories(global_illumination ${GLFW_INCLUDE_DIRS})
include_directories(global_illumination ${GLEW_INCLUDE_DIRS})

target_link_libraries(global_illumination ${LENS_TRACE_LIBRARY} ${LENS_TRACE_CUDA_LIBRARY} ${LENS_TRACE_OPENCL_LIBRARY})
target_link_libraries(global_illumination ${OPENGL_LIBRARIES})
target_link_libraries(global_illumination glfw)
target_link_libraries(global_illumination ${GLEW_LIBRARIES})

add_custom_target(copy_resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
)

add_dependencies(global_illumination copy_resources)