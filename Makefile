CXX = g++
CXXFLAGS = -std=c++17 -Wall
CUDACXX = nvcc
CUDACXXFLAGS = -std=c++17

UNAME := $(shell uname)
HOST := $(shell hostname | cut -c1-4)
ifeq ($(UNAME), Darwin)
# NOTE(allenpthuang): homebrew hack; need more sophisticated mechanisms.
LDFLAGS += -I/opt/homebrew/opt/openssl@3/include
LDFLAGS += -L/opt/homebrew/opt/openssl@3/lib
endif

ifeq ($(HOST), cuda)
	CPU_LDFLAGS += -static-libstdc++
	CUDACXXFLAGS += -Xcompiler -static-libstdc++
endif

LDFLAGS += -lcrypto

TARGET = merkle_tree_demo
BENCHMARK_TARGET = benchmark_cpu
BENCHMARK_TARGET_GPU = benchmark_gpu
TARGET_GPU = merkle_tree_gpu_demo
PATH_OF_CPU_VER = merkle_tree_cpu
PATH_OF_GPU_VER = merkle_tree_gpu
BIN_DIR = ./bin

all: cpu gpu

cpu : demo_cpu benchmark_cpu
gpu : demo_gpu benchmark_gpu

demo_cpu : $(PATH_OF_CPU_VER)/$(PATH_OF_CPU_VER).cpp
	test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	if [[ "$(HOST)" == "cuda" ]]; then module load gcc-11.2; fi; \
	$(CXX) $(CXXFLAGS) -o bin/$(TARGET) \
	$(PATH_OF_CPU_VER)/$(PATH_OF_CPU_VER).cpp \
	$(PATH_OF_CPU_VER)/$(TARGET).cpp $(LDFLAGS) $(CPU_LDFLAGS)

benchmark_cpu : $(PATH_OF_CPU_VER)/$(PATH_OF_CPU_VER).cpp
	test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	if [[ "$(HOST)" == "cuda" ]]; then module load gcc-11.2; fi; \
	$(CXX) $(CXXFLAGS) -o bin/$(BENCHMARK_TARGET) \
	$(PATH_OF_CPU_VER)/$(PATH_OF_CPU_VER).cpp \
	$(PATH_OF_CPU_VER)/$(BENCHMARK_TARGET).cpp $(LDFLAGS) $(CPU_LDFLAGS)

demo_gpu : $(PATH_OF_GPU_VER)/$(PATH_OF_GPU_VER).cu
	test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	if [[ "$(HOST)" == "cuda" ]]; then module load cuda-11.4 gcc-11.2; fi; \
	$(CUDACXX) $(CUDACXXFLAGS) -o bin/$(TARGET_GPU) \
	$(PATH_OF_GPU_VER)/$(PATH_OF_GPU_VER).cu \
	$(PATH_OF_GPU_VER)/$(TARGET_GPU).cu $(LDFLAGS)

benchmark_gpu : $(PATH_OF_GPU_VER)/$(PATH_OF_GPU_VER).cu
	test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	if [[ "$(HOST)" == "cuda" ]]; then module load cuda-11.4 gcc-11.2; fi; \
	$(CUDACXX) $(CUDACXXFLAGS) -o bin/$(BENCHMARK_TARGET_GPU) \
	$(PATH_OF_GPU_VER)/$(PATH_OF_GPU_VER).cu \
	$(PATH_OF_GPU_VER)/$(BENCHMARK_TARGET_GPU).cu $(LDFLAGS) 

