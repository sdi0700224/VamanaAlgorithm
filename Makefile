# Compiler and flags
CXX = g++
CXXFLAGS = -Iinclude -std=c++23 -O3 -MMD -MP -g -Wall -Werror -Wunused -Wextra

# Directories
SRC_DIR = src
TEST_DIR = test
BUILD_DIR = obj
BIN_DIR = bin
DATA_DIR = data

# Files
FILES = query_report.txt stitched_vamana.log

# Google Test directories
GTEST_DIR = third_party/googletest
GTEST_SRC = $(GTEST_DIR)/googletest/src/gtest-all.cc
GTEST_MAIN_SRC = $(GTEST_DIR)/googletest/src/gtest_main.cc
GTEST_INC = $(GTEST_DIR)/googletest/include $(GTEST_DIR)/googletest $(GTEST_DIR)/googletest/src

# Source files
SOURCES = $(SRC_DIR)/ConsoleApp.cpp $(SRC_DIR)/DataLoader.cpp $(SRC_DIR)/ArgumentParser.cpp
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJECTS:.o=.d)

# Exclude ConsoleApp.o from test dependencies
NON_APP_OBJECTS = $(filter-out $(BUILD_DIR)/ConsoleApp.o, $(OBJECTS))

# Test files
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TEST_DEPS = $(TEST_OBJECTS:.o=.d)

# Google Test objects
GTEST_OBJECT = $(BUILD_DIR)/gtest-all.o
GTEST_MAIN_OBJECT = $(BUILD_DIR)/gtest_main.o

# Targets
TEST_TARGET = $(BIN_DIR)/tests
TARGET = $(BIN_DIR)/ConsoleApp

# Dataset directory
DATASET = dummy
DATATYPE =

# Arguments for the executable
K = 50
L = 100
R = 30
A = 1.2
BASE_DATASET = $(DATA_DIR)/$(DATASET)-data$(DATATYPE).bin
QUERY_DATASET = $(DATA_DIR)/$(DATASET)-queries$(DATATYPE).bin
GROUND_TRUTH = $(DATA_DIR)/$(DATASET)-gt$(DATATYPE).bin

# Add Google Test include directories
CXXFLAGS += $(addprefix -I, $(GTEST_INC))

# Build rules
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build Google Test objects
$(GTEST_OBJECT): $(GTEST_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(GTEST_MAIN_OBJECT): $(GTEST_MAIN_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Build tests
test-build: $(TEST_OBJECTS) $(NON_APP_OBJECTS) $(GTEST_OBJECT) $(GTEST_MAIN_OBJECT)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $^ -pthread
	@echo "Tests built successfully."

# Run tests
test-run: $(TEST_TARGET)
	@if [ -n "$(TEST_FILTER)" ]; then \
		./$(TEST_TARGET) --gtest_filter=$(TEST_FILTER); \
	else \
		./$(TEST_TARGET); \
	fi

# Compile source files and generate .d files for dependencies
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Compile test files and generate .d files for dependencies
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(FILES)

# Run the program with arguments
run: $(TARGET)
	./$(TARGET) $(K) $(L) $(R) $(A) $(BASE_DATASET) $(QUERY_DATASET) $(GROUND_TRUTH)

# Debug with Valgrind and run with arguments
debug: CXXFLAGS += -O0 -g
debug: clean $(TARGET)
	valgrind --leak-check=full ./$(TARGET) $(K) $(L) $(R) $(A) $(BASE_DATASET) $(QUERY_DATASET) $(GROUND_TRUTH)

# Include the dependency files if they exist
-include $(DEPS) $(TEST_DEPS)

.PHONY: all clean run debug test test-run
