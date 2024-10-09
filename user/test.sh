#!/bin/bash

# Navigate to the kernel source directory
cd /path/to/kernel/source || exit

# Clean previous builds (optional)
make clean

# Build the kernel
make -j$(nproc)  # Use all available CPU cores

