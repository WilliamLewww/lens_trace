CUDA_PATH = /opt/cuda
OPENCL_SDK_PATH = /usr/local/OpenCL-SDK

EXEC = main.out

SRCS := $(wildcard src/*.cpp)
OBJS := $(notdir $(SRCS:%.cpp=%.o))

CUDA_SRCS := $(wildcard src/kernels/*.cu)
CUDA_OBJS := $(notdir $(CUDA_SRCS:%.cu=%.o))

CFLAGS =-I$(OPENCL_SDK_PATH)/include/api -I$(CUDA_PATH)/include
LDFLAGS =-L$(OPENCL_SDK_PATH)/build -L$(CUDA_PATH)/lib64 -lOpenCL -lcuda -lcudart

$EXEC: $(CUDA_OBJS) $(OBJS)
	g++ $(CFLAGS) -o bin/$(EXEC) build/*.o $(LDFLAGS)

%.o: src/%.cpp
	g++ $(CFLAGS) -c $^ -o build/$@

%.o: src/kernels/%.cu
	$(CUDA_PATH)/bin/nvcc $(CFLAGS) -c $^ -o build/$@