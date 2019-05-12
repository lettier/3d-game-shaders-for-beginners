#!/usr/bin/env bash

SCRIPT_PATH="$(cd "$(dirname "$0")"; pwd -P)"

g++ \
  -Wfatal-errors \
  -c $SCRIPT_PATH/src/main.cxx \
  -o $SCRIPT_PATH/3d-game-shaders-for-beginners.o \
  -std=gnu++11 \
  -O3 \
  -I/usr/include/python3.9/ \
  -I$P3D_INCLUDE_PATH

g++ \
  $SCRIPT_PATH/3d-game-shaders-for-beginners.o \
  -o $SCRIPT_PATH/3d-game-shaders-for-beginners \
  -L$P3D_LIB_PATH \
  -lp3framework \
  -lpanda \
  -lpandafx \
  -lpandaexpress \
  -lpandaphysics \
  -lp3dtoolconfig \
  -lp3dtool \
  -lpthread
