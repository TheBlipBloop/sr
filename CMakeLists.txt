# Declare minimum CMAKE version this project is compatible with.
cmake_minimum_required(VERSION 3.10)

# Declare project name and version.
project(ShaderRenderer
	LANGUAGES CXX
	VERSION 0.0.1
	DESCRIPTION "Local light weight shader display."
	)

# Set C++ standard to C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Header files
set(HEADER_FILES "")

# Source files
set(SOURCE_FILES "src/main.cpp")

# Add executable
add_executable(sr ${SOURCE_FILES} ${HEADER_FILES})
