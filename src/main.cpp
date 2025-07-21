#include "shader.h"
#include <SDL3/SDL_mouse.h>
#include <complex>
#include <cstdint>
#include <string>
#define SDL_MAIN_USE_CALLBACKS
#define GLSL_ENTRY_POINT "main"

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <shader.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>

using namespace std::filesystem;

SDL_GPUDevice* GraphicsDevice;
SDL_Window* Window;
SDL_GPUGraphicsPipeline* Pipeline = nullptr;

path VertexShaderFilePath = "vertex.glsl";
path FragmentShaderFilePath = "shader.glsl";

file_time_type lastGenerateTime;

Shader* FragmentShader = nullptr;
Shader* VertexShader = nullptr;

float testTime = 0.0f;

bool InitializeDeviceAndWindow(const uint window_width,
                               const uint window_height)
{
    SDL_Init(SDL_INIT_VIDEO);

    GraphicsDevice =
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);

    if (GraphicsDevice == nullptr)
    {
        SDL_Log("CreateDevice failed.");
        return false;
    }

    Window = SDL_CreateWindow("Display", window_width, window_height,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP);
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

SDL_GPUGraphicsPipeline* CreateGraphicsPipeline(SDL_GPUDevice* device,
                                                SDL_GPUShader* vertexShader,
                                                SDL_GPUShader* fragmentShader)
{
    assert(device);
    assert(vertexShader);
    assert(fragmentShader);

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
    pipelineCreateInfo.vertex_shader = vertexShader;
    pipelineCreateInfo.fragment_shader = fragmentShader;

    SDL_GPUGraphicsPipeline* pipeline =
        SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);

    if (pipeline == nullptr)
    {
        SDL_Log("CreateGPUGraphicsPipeline failed: %s", SDL_GetError());
    }

    return pipeline;
}

struct alignas(16) UniformBlock
{
    float screen_width;
    float screen_height;
    float iTime;
    float mouse_x;
    float mouse_y;
    int frame;
};

UniformBlock uniform = {0};

bool Draw(SDL_GPUDevice* device, SDL_Window* window,
          SDL_GPUGraphicsPipeline* pipeline)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    if (cmdbuf == nullptr)
    {
        SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            cmdbuf, window, &swapchainTexture, nullptr, nullptr))
    {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        return false;
    }

    if (swapchainTexture != NULL)
    {
        SDL_GPUColorTargetInfo colorTargetInfo = {0};
        colorTargetInfo.texture = swapchainTexture;
        colorTargetInfo.clear_color = (SDL_FColor){1.0f, 0.0f, 1.0f, 1.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderPass =
            SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

        int w;
        int h;
        SDL_GetWindowSize(Window, &w, &h);
        SDL_GetMouseState(&uniform.mouse_x, &uniform.mouse_y);

        uniform.screen_width = w;
        uniform.screen_height = h;

        SDL_PushGPUFragmentUniformData(cmdbuf, 0, &uniform,
                                       sizeof(UniformBlock));

        SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

        uniform.iTime += 1.0f / 32.0f;
        uniform.frame++;

        SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);

    return true;
}

bool RegenerateRenderPipline(Shader* vertex, Shader* fragment)
{
    Pipeline =
        CreateGraphicsPipeline(GraphicsDevice, vertex->Load(GraphicsDevice),
                               fragment->Load(GraphicsDevice, true));

    lastGenerateTime = last_write_time(FragmentShaderFilePath);

    return true;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!InitializeDeviceAndWindow(256, 256))
    {
        SDL_Log("Initialization failed. Aborting.");
        return SDL_AppResult::SDL_APP_FAILURE;
    }

    VertexShader = new Shader("vertex.glsl", ShaderStage::Vertex);
    FragmentShader = new Shader("shader.glsl", ShaderStage::Fragment, 1);

    RegenerateRenderPipline(VertexShader, FragmentShader);

    return SDL_AppResult::SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    SDL_Delay(1.0f / 32.0);

    if (exists(FragmentShaderFilePath) &&
        last_write_time(FragmentShaderFilePath) != lastGenerateTime)
    {
        RegenerateRenderPipline(VertexShader, FragmentShader);
    }

    // If the fragment shdare is invalid
    if (FragmentShader == nullptr)
    {
        // Pause until its valid again
        return SDL_APP_CONTINUE;
    }

    bool drawSuccess = Draw(GraphicsDevice, Window, Pipeline);

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
    SDL_ReleaseGPUGraphicsPipeline(GraphicsDevice, Pipeline);
    SDL_ReleaseWindowFromGPUDevice(GraphicsDevice, Window);
    SDL_DestroyWindow(Window);
    SDL_DestroyGPUDevice(GraphicsDevice);
}
