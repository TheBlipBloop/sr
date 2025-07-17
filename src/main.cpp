#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <shaderc/shaderc.h>
#include <shaderc/status.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <shaderc/shaderc.hpp>

#define SHADER_FILE "./shader.glsl"
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

SDL_GPUDevice *GraphicsDevice;
SDL_Window *Window;

SDL_GPUShader *FragmentShader = nullptr;
SDL_GPUShader *VertexShader = nullptr;
SDL_GPUGraphicsPipeline *Pipeline = nullptr;

bool Initialize(const uint window_width, const uint window_height) {
  SDL_Init(SDL_INIT_VIDEO);

  // Init GPU
  GraphicsDevice =
      SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);

  if (GraphicsDevice == nullptr) {
    SDL_Log("CreateDevice failed.");
    return false;
  }

  Window = SDL_CreateWindow("Display", window_width, window_height,
                            SDL_WINDOW_RESIZABLE);
  if (Window == nullptr) {
    SDL_Log("CreateWindow failed: %s", SDL_GetError());
    return false;
  }

  if (!SDL_ClaimWindowForGPUDevice(GraphicsDevice, Window)) {
    SDL_Log("ClaimWindow failed.");
    return false;
  }

  return true;
}

SDL_GPUGraphicsPipeline *CreateGraphicsPipeline(SDL_GPUDevice *device,
                                                SDL_GPUShader *fragmentShader) {
  // SDL_GPUGraphicsPipelineTargetInfo targetInfo = {0};
  // targetInfo.num_color_targets = 0;

  /*
  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .target_info =
          {
              .num_color_targets = 1,
              .color_target_descriptions =
                  (SDL_GPUColorTargetDescription[]){
                      {.format =
                           SDL_GetGPUSwapchainTextureFormat(device, Window)}},
          },
      .rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL,
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .vertex_shader = nullptr,
      .fragment_shader = fragmentShader,
  };
  */

  /*
  .num_color_targets = 1,
  .color_target_descriptions =
      (SDL_GPUColorTargetDescription[]){
          {.format = SDL_GetGPUSwapchainTextureFormat(device, window)}},
  */

  /*
  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .target_info =
          {
              .num_color_targets = 1,
              .color_target_descriptions =
                  (SDL_GPUColorTargetDescription[]){
                      {.format =
                           SDL_GetGPUSwapchainTextureFormat(device, window)}},
          },

      .rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
  .vertex_shader = vertexShader, .fragment_shader = fragmentShader,
};
*/

  SDL_GPUColorTargetDescription colorTargetInfo = {};
  colorTargetInfo.format = SDL_GetGPUSwapchainTextureFormat(device, Window);

  SDL_GPUGraphicsPipelineTargetInfo targetInfo = {};
  targetInfo.has_depth_stencil_target = false;
  targetInfo.color_target_descriptions = &colorTargetInfo;
  targetInfo.num_color_targets = 1;

  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {0};
  pipelineCreateInfo.target_info = targetInfo;
  pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
  pipelineCreateInfo.fragment_shader = fragmentShader;
  pipelineCreateInfo.vertex_shader = VertexShader;

  if (fragmentShader == nullptr) {
    SDL_Log("no frag shader");
  }
  if (VertexShader == nullptr) {
    SDL_Log("no vert shader");
  }

  /*
  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .target_info = {
          .num_color_targets = 1,
          .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
              .format = SDL_GetGPUSwapchainTextureFormat(device, window)
          }},
      },

      .rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
  };
  */

  SDL_GPUGraphicsPipeline *pipeline =
      SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);

  if (pipeline == nullptr) {
    SDL_Log("CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
  }

  return pipeline;
}

bool Draw() {
  SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(GraphicsDevice);
  if (cmdbuf == nullptr) {
    SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
    return false;
  }

  SDL_GPUTexture *swapchainTexture;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, Window, &swapchainTexture,
                                             nullptr, nullptr)) {
    SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
    return false;
  }

  if (swapchainTexture != NULL) {
    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){1.0f, 0.0f, 1.0f, 1.0f};
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass *renderPass =
        SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);
  }
  SDL_SubmitGPUCommandBuffer(cmdbuf);

  return true;
  /*
  SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(GraphicsDevice);
  if (cmdbuf == nullptr)
  {
      SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
      return false;
  }

  SDL_GPUTexture* swapchainTexture;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(
          cmdbuf, Window, &swapchainTexture, nullptr, nullptr))
  {
      SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
      return false;
  }

  SDL_GPUColorTargetInfo colorTargetInfo = {0};
  colorTargetInfo.texture = swapchainTexture;
  colorTargetInfo.clear_color = (SDL_FColor){0.3f, 0.1f, 0.5f, 1.0f};
  colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
  colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

  SDL_GPURenderPass* renderPass =
      SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
  SDL_EndGPURenderPass(renderPass);

  SDL_SubmitGPUCommandBuffer(cmdbuf);
  return true;
*/
}

bool CompileGLSL(const char *program, size_t programSize, const bool vertex,
                 char **output, size_t *outputSize) {
  shaderc_compiler_t glslCompiler = shaderc_compiler_initialize();
  shaderc_compilation_result_t result = shaderc_compile_into_spv(
      glslCompiler, program, programSize,
      vertex ? shaderc_glsl_vertex_shader : shaderc_glsl_fragment_shader,
      vertex ? "vertex.vert" : "shader.frag", "main", nullptr);

  const auto status = shaderc_result_get_compilation_status(result);

  if (status == shaderc_compilation_status_success) {
    const char *bytes = shaderc_result_get_bytes(result);
    const size_t size = shaderc_result_get_length(result);

    *output = (char *)malloc(size);
    memcpy(*output, bytes, size);

    shaderc_result_release(result);
    *outputSize = size;

    SDL_Log("Compiled shader of length : %zd", size);

    return true;
  }

  const size_t errors = shaderc_result_get_num_errors(result);

  SDL_Log("Failed to compile GLSL code. Encountered %zd errors.", errors);

  SDL_Log("%s", shaderc_result_get_error_message(result));

  shaderc_result_release(result);
  return false;
}

SDL_GPUShader *LoadShaderFromGLSL(SDL_GPUDevice *device,
                                  const char *shaderFilePath,
                                  const bool vertex) {
  size_t fileSize;
  const char *file = (char *)SDL_LoadFile(shaderFilePath, &fileSize);

  char *shaderCode = nullptr;
  size_t shaderCodeSize;
  const bool compileSuccess =
      CompileGLSL(file, fileSize, vertex, &shaderCode, &shaderCodeSize);

  if (!compileSuccess) {
    return nullptr;
  }

  SDL_GPUShaderCreateInfo info = {0};
  info.code = (Uint8 *)shaderCode;
  info.code_size = shaderCodeSize;
  info.entrypoint = "main";
  info.format = SDL_GPU_SHADERFORMAT_SPIRV;
  info.stage =
      vertex ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;

  return SDL_CreateGPUShader(device, &info);
}

SDL_GPUShader *LoadShaderRaw(SDL_GPUDevice *device, const char *shaderFilePath,
                             const bool vertex) {
  size_t fileSize;
  const char *file = (char *)SDL_LoadFile(shaderFilePath, &fileSize);

  SDL_GPUShaderCreateInfo info = {0};
  info.code = (Uint8 *)file;
  info.code_size = fileSize;
  info.entrypoint = "main";
  info.format = SDL_GPU_SHADERFORMAT_SPIRV;
  info.stage =
      vertex ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;

  return SDL_CreateGPUShader(device, &info);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  if (!Initialize(256, 256)) {
    SDL_Log("Initialization failed. Aborting.");
    return SDL_AppResult::SDL_APP_FAILURE;
  }

  // FragmentShader = LoadShaderFromGLSL(GraphicsDevice, "shader.glsl", false);
  // VertexShader = LoadShaderFromGLSL(GraphicsDevice, "vertex.glsl", true);

  VertexShader = LoadShaderRaw(GraphicsDevice, "v.spv", true);
  FragmentShader = LoadShaderRaw(GraphicsDevice, "f.spv", false);

  Pipeline = CreateGraphicsPipeline(GraphicsDevice, FragmentShader);

  return SDL_AppResult::SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  bool drawSuccess = Draw();

  return drawSuccess ? SDL_AppResult::SDL_APP_CONTINUE
                     : SDL_AppResult::SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_AppResult::SDL_APP_FAILURE;
  }

  return SDL_AppResult::SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  SDL_WaitForGPUIdle(GraphicsDevice);
  SDL_ReleaseWindowFromGPUDevice(GraphicsDevice, Window);
  SDL_DestroyWindow(Window);
  SDL_DestroyGPUDevice(GraphicsDevice);
}
