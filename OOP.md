# Refactor => OOP

Right now the codebase is very clike which is great but its C++20 so we might as well take advantage of classes and all that --bloat-- fancy shit.

!!! UPDATE : Pipelines are the ones extended for hot reloading, shaders simply implement a Load<>(gpu) method to place shader on gpu

## Project structure breakdown

### Renderer.cpp/h

This class is a mix of the Dispaly and Pipeline classes. Basically responsible for setting up the initial window and managing GPU devices, etc. Also implements the Draw method. Definitlity bloated but eh such is graphics.

- Initialize()
  - InitializeWindow()
  - InitializeDevice()
- Draw()
- Quit()

### Shader.cpp/h

Contain the shader base class. Responsible for loading the shader file from the disk and compiling it. Also stores uniform buffers, etc.

### HotreloadShader.cpp/h

A shader that automatically checks for changes from its source file. Checking either occurs on fetch of the GPU shader resource or via some other update mechanism.

### Display

Initializes and manages window and GPU devices.

### Pipeline

Responsbile for creating and maintaining render pipeline. Also must regenerate the pipeline as needed.

- GetPipeline()
- UpdateShader(Shader\* shade, ShaderStage stage);

### main.cpp

Entry point. Driver code. Uses SDL callbacks.

Thats all for now.

Ok so, right now we have a basic shader class and should probably put the rest of the stuff into a nice renderer class but like we don't need to.

It feels a bit jank but I want to just keep it as is and add mroe features to the shader class
