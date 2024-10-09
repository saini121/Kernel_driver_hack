#!/bin/bash

# Navigate to the kernel source directory
cd ./kernel || exit  # Adjust this to your actual kernel source directory

# Clean previous builds (optional)
make clean

# Build the kernel
make -j$(nproc)  # Use all available CPU cores
