# Roadmap

- Set up crossshader libraries
  - alright so shadercross is a pain in the ass to build for so so many reasons theres also no doucmnetation for it and it blows
  - I think it might be better to use opengl to compile GLSL/HLSL => SPRIRV => render with SDL GPU
  - glslang should do the trick -- bit over kill but should get us what we neeeed
- Set up compilation of HLSL => realtime GPU shader
- Render GPU shader on screen @ real time
- Inject uniforms (time, mouse positions, etc)
- Watch for file changes and auto reload shader
