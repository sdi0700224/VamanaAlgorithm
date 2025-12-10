# Compiler and flags
CXX = g++
CXXFLAGS = -Iinclude -std=c++17 -O3 -MMD -MP -g -Wall -Werror -Wunused -Wextra -fopenmp

# Directories
SRC_DIR = src
TEST_DIR = test
BUILD_DIR = obj
BIN_DIR = bin
DATA_DIR = data
INDEX_DIR = indexes
EXP_DIR = experiments

# Files
FILES = *.log

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

# Standalone program
GENERATOR_SRC = $(SRC_DIR)/GtGenerator.cpp
GENERATOR_OBJ = $(BUILD_DIR)/GtGenerator.o
GENERATOR_DEP = $(GENERATOR_OBJ:.o=.d)
GENERATOR_TARGET = $(BIN_DIR)/GtGenerator

# Targets
TEST_TARGET = $(BIN_DIR)/tests
TARGET = $(BIN_DIR)/ConsoleApp

# Dataset directory
DATASET = dummy#dummy,contest
DATATYPE =#-release-1m
GRAPH_NAME = f_50_index
EXP_NAME = f_50_exp

# Arguments for the executable
K = 50
L = 100
R = 60#30
A = 1.2#1.1
OPERATION = search	#create-f,create-s,search

BASE_DATASET = $(DATA_DIR)/$(DATASET)-data$(DATATYPE).bin
QUERY_DATASET = $(DATA_DIR)/$(DATASET)-queries$(DATATYPE).bin
GROUND_TRUTH = $(DATA_DIR)/$(DATASET)-gt$(DATATYPE).bin
GRAPH = $(INDEX_DIR)/$(GRAPH_NAME).bin
EXPERIMENT = $(EXP_DIR)/$(EXP_NAME)

# Add Google Test include directories
CXXFLAGS += $(addprefix -I, $(GTEST_INC))

# Build rules
all: ensure-dirs $(TARGET) $(GENERATOR_TARGET)

# Ensure necessary directories exist
ensure-dirs:
	@mkdir -p $(DATA_DIR)
	@mkdir -p $(EXP_DIR)
	@mkdir -p $(INDEX_DIR)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(GENERATOR_TARGET): $(GENERATOR_OBJ)
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

# Compile GtGenerator and generate .d file
$(BUILD_DIR)/GtGenerator.o: $(SRC_DIR)/GtGenerator.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(FILES)

# Clean experiment files
clean-exp:
	rm -rf $(EXP_DIR)

# Clean index files
clean-ind:
	rm -rf $(INDEX_DIR)

# Clean all
clean-all: clean clean-exp clean-ind

# Run the program with arguments
run: ensure-dirs $(TARGET)
	./$(TARGET) $(K) $(L) $(R) $(A) $(BASE_DATASET) $(QUERY_DATASET) $(GROUND_TRUTH) $(OPERATION) $(GRAPH) $(EXPERIMENT)

# Run GtGenerator
run-generator: ensure-dirs $(GENERATOR_TARGET)
	./$(GENERATOR_TARGET) $(BASE_DATASET) $(QUERY_DATASET) $(GROUND_TRUTH)

# Debug with Valgrind and run with arguments
debug: CXXFLAGS += -O0 -g
debug: clean $(TARGET)
	valgrind --leak-check=full ./$(TARGET) $(K) $(L) $(R) $(A) $(BASE_DATASET) $(QUERY_DATASET) $(GROUND_TRUTH) $(OPERATION) $(GRAPH) $(EXPERIMENT)

# Include the dependency files if they exist
-include $(DEPS) $(TEST_DEPS) $(GENERATOR_DEP)

.PHONY: all clean run debug test test-run run-generator ensure-dirs
