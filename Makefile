CXX = g++
CXXFLAGS = -std=c++17 -Wall
# NOTE(allenpthuang): homebrew hack; need more sophisticated mechanisms.
LDFLAGS = -I/opt/homebrew/opt/openssl@3/include
LDFLAGS += -L/opt/homebrew/opt/openssl@3/lib
LDFLAGS += -lcrypto
TARGET = merkle_tree_demo
PATH_OF_CPU_VER = merkle_tree_cpu
BIN_DIR = ./bin

all: cpu 

cpu :
	test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o bin/$(TARGET) \
	$(PATH_OF_CPU_VER)/$(PATH_OF_CPU_VER).cpp \
	$(PATH_OF_CPU_VER)/$(TARGET).cpp $(LDFLAGS)
