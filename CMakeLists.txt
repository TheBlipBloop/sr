# Declare minimum CMAKE version this project is compatible with.
cmake_minimum_required(VERSION 3.10)

# Declare project name and version.
project(ShaderRenderer
	LANGUAGES CXX
	VERSION 0.0.1
	DESCRIPTION "Local light weight shader display."
	)

# Set C++ standard to C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find SDL3 & OpenGL
find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

# ? not needed any more ?
set(SHADERC_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)

#
add_subdirectory(external/shaderc)

# Header files
set(HEADER_FILES "src/shader.h")

# Source files
set(SOURCE_FILES "src/main.cpp" "src/shader.cpp")

# Define includes
include_directories(${SDL3_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} external/shaderc)

# Add executable
add_executable(sr ${SOURCE_FILES} ${HEADER_FILES})

# Link libs
target_link_libraries(sr
	${SDL3_LIBRARIES}
	${OpenGL_LIBRARIES}
	shaderc_combined)
