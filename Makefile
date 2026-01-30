# CHIP-8 Emulator Makefile

# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS := -lraylib -lm -lpthread -ldl -lrt

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# Files
TARGET := $(BIN_DIR)/chip8-emulator
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

# Platform detection
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lGL -lX11
endif

ifeq ($(UNAME_S),Darwin)
    LDFLAGS := -lraylib -framework IOKit -framework Cocoa -framework OpenGL
endif

# Default target
all: directories $(TARGET)

# Create directories
directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Link
$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile with dependency generation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependencies
-include $(DEPS)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

# Run the emulator (requires ROM path)
run: $(TARGET)
	@if [ -z "$(ROM)" ]; then \
		echo "Usage: make run ROM=path/to/rom.ch8"; \
	else \
		$(TARGET) $(ROM); \
	fi

# Help
help:
	@echo "CHIP-8 Emulator Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all (default) - Build the emulator"
	@echo "  clean         - Remove build artifacts"
	@echo "  run ROM=<path> - Build and run with specified ROM"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make run ROM=roms/pong.ch8"
	@echo "  make clean"

.PHONY: all clean run help directories
