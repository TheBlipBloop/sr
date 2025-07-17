#!/bin/bash

cd build
make -j 24
cd ..
./build/sr
