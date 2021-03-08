CUDA_BIN_PATH = /opt/cuda/bin
OPENCL_SDK_PATH = /usr/local/OpenCL-SDK

EXEC = main.out

SRCS := $(wildcard src/*.cpp)
OBJS := $(notdir $(SRCS:%.cpp=%.o))

CFLAGS =-I$(OPENCL_SDK_PATH)/include/api
LDFLAGS =-L$(OPENCL_SDK_PATH)/build -lOpenCL

$EXEC: $(OBJS)
	g++ $(CFLAGS) -o bin/$(EXEC) build/*.o $(LDFLAGS)

%.o: src/%.cpp
	g++ $(CFLAGS) -c $^ -o build/$@