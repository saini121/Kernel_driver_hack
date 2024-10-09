#!/bin/bash

# Navigate to the kernel source directory
cd /home/runner/work/android_kernel_oplus_RMX3461 || exit 1

# Clean previous builds and compile
make clean && make

# Check if the binary is created
if [[ -f "main" ]]; then
    echo "Kernel compiled successfully."
else
    echo "Kernel compilation failed."
    exit 1
fi

# Push the binary to the specified location
adb push main /data/local/tmp
# Uncomment if needed
# adb shell ./data/local/tmp/main
