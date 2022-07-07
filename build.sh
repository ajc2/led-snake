#!/bin/sh
export PATH="$HOME/Downloads/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/bin:$PATH"
mkdir build
cd build
cmake ..
make
