#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <cstddef>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <iostream>

#define SHADER_FILE "./shader.glsl"
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

// Current state of this program.
struct State
{
};

SDL_GPUGraphicsPipeline* FragmentShaderPipeline;
SDL_GPUTexture* d;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
  SDL_Init(SDL_INIT_VIDEO);

  // Init GPU
  SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV |
                                                  SDL_GPU_SHADERFORMAT_DXIL |
                                                  SDL_GPU_SHADERFORMAT_MSL,
                                              true, nullptr);

  if (device == nullptr)
  {
    SDL_Log("CreateDevice failed.");
    return SDL_AppResult::SDL_APP_FAILURE;
  }

  SDL_Window* window =
      SDL_CreateWindow("Display", 250, 250, SDL_WINDOW_RESIZABLE);
  if (window == nullptr)
  {
    SDL_Log("CreateWindow failed: %s", SDL_GetError());
    return SDL_AppResult::SDL_APP_FAILURE;
  }

  if (!SDL_ClaimWindowForGPUDevice(device, window))
  {
    SDL_Log("ClaimWindow failed.");
    return SDL_AppResult::SDL_APP_FAILURE;
  }

  SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
  if (cmdbuf == nullptr)
  {
    SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
    return SDL_AppResult::SDL_APP_FAILURE;
  }

  SDL_GPUTexture* swapchainTexture;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, window, &swapchainTexture,
                                             nullptr, nullptr))
  {
    SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
    return SDL_AppResult::SDL_APP_FAILURE;
  }

  SDL_GPUColorTargetInfo colorTargetInfo = {0};
  colorTargetInfo.texture = swapchainTexture;
  colorTargetInfo.clear_color = (SDL_FColor){0.3f, 0.6f, 0.5f, 1.0f};
  colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
  colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

  SDL_GPURenderPass* renderPass =
      SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
  SDL_EndGPURenderPass(renderPass);

  SDL_SubmitGPUCommandBuffer(cmdbuf);

  SDL_Log("WAHOO!");
  /*
  SDL_Delay(10000);


  SDL_WaitForGPUIdle(device);
  SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyWindow(window);
  SDL_DestroyGPUDevice(device);
*/
  return SDL_AppResult::SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
  return SDL_AppResult::SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
  if (event->type == SDL_EVENT_QUIT)
  {
    return SDL_AppResult::SDL_APP_FAILURE;
  }

  return SDL_AppResult::SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
