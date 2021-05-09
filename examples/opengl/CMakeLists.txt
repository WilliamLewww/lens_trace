cmake_minimum_required (VERSION 2.8.12)
project(example_opengl LANGUAGES CXX)

find_path(LENS_TRACE_INCLUDE_DIRECTORY
	NAMES lens_trace
	HINTS /usr/local/include/lens_trace
)

find_library(LENS_TRACE_LIBRARY
	NAMES liblenstrace lenstrace
	HINTS /usr/local/lib/lens_trace
)

add_executable(example_opengl main.cpp)
include_directories(example_opengl ${LENS_TRACE_INCLUDE_DIRECTORY})
target_link_libraries(example_opengl ${LENS_TRACE_LIBRARY})