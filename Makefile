CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# Target executable
TARGET = chip8

# Source files
SOURCES = main.cpp interpreter.cpp peripheral.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(SFML_LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run the emulator
run: $(TARGET)
	./$(TARGET)

# Install SFML (macOS with Homebrew)
install-sfml:
	brew install sfml

# Install SFML (Ubuntu/Debian)
install-sfml-ubuntu:
	sudo apt-get update
	sudo apt-get install libsfml-dev

.PHONY: all clean run install-sfml install-sfml-ubuntu

