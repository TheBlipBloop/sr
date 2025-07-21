#!/bin/bash

# sync submodules 
git submodule update --init --recursive 

# pull in the shaderc deps
./external/shaderc/utils/git-sync-deps

# build cmake files 
cmake -B build 
