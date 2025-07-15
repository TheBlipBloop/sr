#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <ios>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <iostream>

#define SHADER_FILE "./shader.glsl"
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

SDL_GPUDevice* GraphicsDevice;
SDL_Window* Window;

SDL_GPUGraphicsPipeline* FragmentShaderPipeline;
SDL_GPUTexture* d;

bool Initialize(const uint window_width, const uint window_height)
{
    SDL_Init(SDL_INIT_VIDEO);

    // Init GPU
    GraphicsDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV |
                                             SDL_GPU_SHADERFORMAT_DXIL |
                                             SDL_GPU_SHADERFORMAT_MSL,
                                         true, nullptr);

    if (GraphicsDevice == nullptr)
    {
        SDL_Log("CreateDevice failed.");
        return false;
    }

    Window = SDL_CreateWindow("Display", window_width, window_height,
                              SDL_WINDOW_RESIZABLE);
    if (Window == nullptr)
    {
        SDL_Log("CreateWindow failed: %s", SDL_GetError());
        return false;
    }

    if (!SDL_ClaimWindowForGPUDevice(GraphicsDevice, Window))
    {
        SDL_Log("ClaimWindow failed.");
        return false;
    }

    return true;
}

bool Draw()
{
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
}

SDL_GPUGraphicsPipeline* CreatePipelineFromShader(const char* shaderFilePath)
{
    return nullptr;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!Initialize(256, 256))
    {
        SDL_Log("Initialization failed. Aborting.");
        return SDL_AppResult::SDL_APP_FAILURE;
    }

    SDL_Log("WAHOO!");

    return SDL_AppResult::SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    bool drawSuccess = Draw();

    return drawSuccess ? SDL_AppResult::SDL_APP_CONTINUE
                       : SDL_AppResult::SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_AppResult::SDL_APP_FAILURE;
    }

    return SDL_AppResult::SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_WaitForGPUIdle(GraphicsDevice);
    SDL_ReleaseWindowFromGPUDevice(GraphicsDevice, Window);
    SDL_DestroyWindow(Window);
    SDL_DestroyGPUDevice(GraphicsDevice);
}
