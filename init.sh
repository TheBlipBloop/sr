#!/bin/bash

# sync submodules 
git submodules update --init --recursive 

# pull in the shaderc utils 
./external/shaderc/utils/git-sync-deps

