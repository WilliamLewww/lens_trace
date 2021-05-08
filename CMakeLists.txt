cmake_minimum_required (VERSION 2.8.12)
project(lens_trace LANGUAGES CXX)

find_package(CUDA)
find_package(OpenCL)
find_package(GTest)
find_package(Threads)

if(GTest_FOUND)
  enable_testing()
endif()

if(NOT CUDA_FOUND AND NOT OpenCL_FOUND)
  message(FATAL_ERROR "Could not find CUDA or OpenCL. Exiting")
endif()

file(GLOB SOURCES "src/*.cpp" "src/kernels/*.cu")

add_library(lenstrace SHARED
  ${SOURCES}
)
include_directories(lenstrace include)

add_executable(LensTrace src/main.cpp)

if(CUDA_FOUND)
  enable_language(CUDA)
  target_compile_definitions(lenstrace PRIVATE -DCUDA_ENABLED)
  target_compile_definitions(LensTrace PRIVATE -DCUDA_ENABLED)
endif()

if(OpenCL_FOUND)
  target_compile_definitions(lenstrace PRIVATE -DOPENCL_ENABLED)
  target_compile_definitions(LensTrace PRIVATE -DOPENCL_ENABLED)
  target_link_libraries(lenstrace ${OpenCL_LIBRARIES})
endif()

if(GTest_FOUND AND BUILD_TESTING)
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
  if(CUDA_FOUND)
    include_directories(CUDA_RENDERER_TEST include)
    include_directories(CUDA_RENDERER_TEST ${GTEST_INCLUDE_DIRS})
    add_executable(CUDA_RENDERER_TEST tests/cuda_renderer_test.cc)
    target_compile_definitions(CUDA_RENDERER_TEST PRIVATE -DCUDA_ENABLED)
    target_link_libraries(CUDA_RENDERER_TEST ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} lenstrace)
    add_test(CUDA_TESTS tests/CUDA_RENDERER_TEST)
  endif()
  if(OPENCL_FOUND)
    include_directories(OPENCL_RENDERER_TEST include)
    include_directories(OPENCL_RENDERER_TEST ${GTEST_INCLUDE_DIRS})
    add_executable(OPENCL_RENDERER_TEST tests/opencl_renderer_test.cc)
    target_compile_definitions(OPENCL_RENDERER_TEST PRIVATE -DOPENCL_ENABLED)
    target_link_libraries(OPENCL_RENDERER_TEST ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} lenstrace)
    add_test(OPENCL_TESTS tests/OPENCL_RENDERER_TEST)
  endif()
endif()

target_link_libraries(LensTrace lenstrace)

add_custom_target(copy_resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
)

add_dependencies(LensTrace copy_resources)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liblenstrace.so DESTINATION lib/${PROJECT_NAME})

file(GLOB LENS_TRACE_INCLUDE "include/lens_trace/*.h")
file(GLOB NLOHMANN_INCLUDE "include/nlohmann/*.hpp")
file(GLOB STB_INCLUDE "include/stb/*.h")
file(GLOB TINYOBJLOADER_INCLUDE "include/tinyobjloader/*.h")

install(FILES ${LENS_TRACE_INCLUDE} DESTINATION include/${PROJECT_NAME}/lens_trace)
install(FILES ${NLOHMANN_INCLUDE} DESTINATION include/${PROJECT_NAME}/nlohmann)
install(FILES ${STB_INCLUDE} DESTINATION include/${PROJECT_NAME}/stb)
install(FILES ${TINYOBJLOADER_INCLUDE} DESTINATION include/${PROJECT_NAME}/tinyobjloader)