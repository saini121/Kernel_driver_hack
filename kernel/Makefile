# User-space Application Makefile

CC := /home/rog/project/android_kernel/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-g++
CFLAGS := -g -static
TARGET := main
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

# Default target
all: $(TARGET)

# Rule for building object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for linking the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Clean target
clean:
	rm -rf $(TARGET) *.o
