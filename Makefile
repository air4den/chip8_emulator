CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Target executable
TARGET = chip8

# Source files
SOURCES = main.cpp interpreter.cpp peripheral.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default ROM path
ROM ?= ./roms/tests/ibm_logo.ch8

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

# Run the emulator with ROM path
run: $(TARGET)
	./$(TARGET) $(ROM)

.PHONY: all clean run install-sfml install-sfml-ubuntu

