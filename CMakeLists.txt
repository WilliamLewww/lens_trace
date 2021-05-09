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

add_executable(LensTrace src/main.cpp)

file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

file(GLOB SOURCES_CUDA "src/cuda/*.cpp" "src/cuda/kernels/*.cu")
file(GLOB SOURCES_OPENCL "src/opencl/*.cpp")

add_library(lenstrace SHARED
  ${SOURCES}
)
include_directories(lenstrace include)

if(CUDA_FOUND)
  add_library(lenstrace-cuda SHARED
    ${SOURCES_CUDA}
  )
  include_directories(lenstrace-cuda include)

  enable_language(CUDA)
  target_link_libraries(LensTrace lenstrace-cuda)
endif()

if(OpenCL_FOUND)
  add_library(lenstrace-opencl SHARED
    ${SOURCES_OPENCL}
  )
  include_directories(lenstrace-opencl include)

  target_link_libraries(lenstrace-opencl ${OpenCL_LIBRARIES})
  target_link_libraries(LensTrace lenstrace-opencl)
endif()

if(GTest_FOUND AND BUILD_TESTING)
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
  if(CUDA_FOUND)
    add_executable(CUDA_RENDERER_TEST tests/cuda_renderer_test.cc)
    include_directories(CUDA_RENDERER_TEST include)
    include_directories(CUDA_RENDERER_TEST ${GTEST_INCLUDE_DIRS})
    target_link_libraries(CUDA_RENDERER_TEST ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} lenstrace lenstrace-cuda)
    add_test(CUDA_TESTS tests/CUDA_RENDERER_TEST)
  endif()
  if(OPENCL_FOUND)
    add_executable(OPENCL_RENDERER_TEST tests/opencl_renderer_test.cc)
    include_directories(OPENCL_RENDERER_TEST include)
    include_directories(OPENCL_RENDERER_TEST ${GTEST_INCLUDE_DIRS})
    target_link_libraries(OPENCL_RENDERER_TEST ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} lenstrace lenstrace-opencl)
    add_test(OPENCL_TESTS tests/OPENCL_RENDERER_TEST)
  endif()
endif()

target_link_libraries(LensTrace lenstrace)

add_custom_target(copy_resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
)

add_dependencies(LensTrace copy_resources)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liblenstrace.so DESTINATION lib/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liblenstrace-cuda.so DESTINATION lib/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liblenstrace-opencl.so DESTINATION lib/${PROJECT_NAME})

file(GLOB LENS_TRACE_INCLUDE "include/lens_trace/*.h")
file(GLOB LENS_TRACE_CUDA_INCLUDE "include/lens_trace/cuda/*.h")
file(GLOB LENS_TRACE_OPENCL_INCLUDE "include/lens_trace/opencl/*.h")
file(GLOB LENS_TRACE_OPTIX_INCLUDE "include/lens_trace/optix/*.h")

file(GLOB NLOHMANN_INCLUDE "include/nlohmann/*.hpp")
file(GLOB STB_INCLUDE "include/stb/*.h")
file(GLOB TINYOBJLOADER_INCLUDE "include/tinyobjloader/*.h")

install(FILES ${LENS_TRACE_INCLUDE} DESTINATION include/${PROJECT_NAME}/lens_trace)
install(FILES ${LENS_TRACE_CUDA_INCLUDE} DESTINATION include/${PROJECT_NAME}/lens_trace/cuda)
install(FILES ${LENS_TRACE_OPENCL_INCLUDE} DESTINATION include/${PROJECT_NAME}/lens_trace/opencl)
install(FILES ${LENS_TRACE_OPTIX_INCLUDE} DESTINATION include/${PROJECT_NAME}/lens_trace/optix)

install(FILES ${NLOHMANN_INCLUDE} DESTINATION include/${PROJECT_NAME}/nlohmann)
install(FILES ${STB_INCLUDE} DESTINATION include/${PROJECT_NAME}/stb)
install(FILES ${TINYOBJLOADER_INCLUDE} DESTINATION include/${PROJECT_NAME}/tinyobjloader)

install(DIRECTORY resources DESTINATION share/${PROJECT_NAME})