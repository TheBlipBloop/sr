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

# Find SDL3, OpenGL, and GLEW
find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)


# Header files
set(HEADER_FILES "")

# Source files
set(SOURCE_FILES "src/main.cpp")

# Define includes
include_directories(${SDL3_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})

# Add executable
add_executable(sr ${SOURCE_FILES} ${HEADER_FILES})

# Link libs
target_link_libraries(sr
	${SDL3_LIBRARIES}
	${OpenGL_LIBRARIES}
	${GLEW_LIBRARIES})

