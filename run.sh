#!/bin/sh

clear
rm ./TAL-Test

# To build it
mkdir build
cd build
cmake ..
make

echo " ------------------------"
echo "Start Running the app:"

# To Run it
cd ..
./TAL-Test