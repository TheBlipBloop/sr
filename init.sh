#!/bin/bash

# sync submodules 
git submodule update --init --recursive 

# pull in the shaderc utils 
./external/shaderc/utils/git-sync-deps

