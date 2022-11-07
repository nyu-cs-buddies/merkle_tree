CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDFLAGS = -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lcrypto
TARGET = merkle_tree_cpu
BIN_DIR = ./bin

all: $(TARGET)

$(TARGET) : $(TARGET)/$(TARGET).cpp
	test -d $(BIN_DIR) || mkdir $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o bin/$(TARGET) $(TARGET)/$(TARGET).cpp $(LDFLAGS)
