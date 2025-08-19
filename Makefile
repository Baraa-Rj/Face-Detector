# Makefile for Face Detection with OpenCV
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
OPENCV_LIBS = $(shell pkg-config --libs opencv4)
OPENCV_CFLAGS = $(shell pkg-config --cflags opencv4)

# Target executable
TARGET = face_detection

# Source files
SOURCES = main.cpp

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(OPENCV_CFLAGS) -o $(TARGET) $(SOURCES) $(OPENCV_LIBS)

# Clean build files
clean:
	rm -f $(TARGET)

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libopencv-dev pkg-config

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean install-deps run
