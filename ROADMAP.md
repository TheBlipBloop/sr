# Roadmap

- Set up crossshader libraries
  - alright so shadercross is a pain in the ass to build for so so many reasons theres also no doucmnetation for it and it blows
  - I think it might be better to use opengl to compile GLSL/HLSL => SPRIRV => render with SDL GPU
  - glslang should do the trick -- bit over kill but should get us what we neeeed
  - ended up using shaderc a google wrapper around glsl and it works!! we have shader code in RAM!
- Set up compilation of HLSL => realtime GPU shader
  - done! well using GLSL at least we are able to generate a GPU shader using SDLgpu and shaderc

- Render GPU shader on screen @ real time
  - we are going to need to figure out
    - render pipelines
    - quads
    - vertex buffers
    - uhh
- Inject uniforms (time, mouse positions, etc)
- Watch for file changes and auto reload shader
- Record videos and screenshot interface
