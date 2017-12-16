#!/bin/sh

clear
# Remove the binaries.
rm -rf build/tests/*

# build it.
mkdir build
cd build
cmake ..
make

echo " ------------------------"
echo "Build finished"
echo " "