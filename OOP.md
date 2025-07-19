# Refactor => OOP

Right now the codebase is very clike which is great but its C++20 so we might as well take advantage of classes and all that --bloat-- fancy shit.

!!! UPDATE : Pipelines are the ones extended for hot reloading, shaders simply implement a Load<>(gpu) method to place shader on gpu

## Project structure breakdown

### Shader.cpp/h

Contain the shader base class. Responsible for loading the shader file from the disk and compiling it. Also stores uniform buffers, etc.

### HotreloadShader.cpp/h

A shader that automatically checks for changes from its source file. Checking either occurs on fetch of the GPU shader resource or via some other update mechanism

### Display

Initializes and manages window and GPU devices.

### Pipeline?

idk

### main.cpp

Entry point. Driver code. Uses SDL callbacks.

Thats all for now.
