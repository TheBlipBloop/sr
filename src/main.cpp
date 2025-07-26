#define SDL_MAIN_USE_CALLBACKS
#define APPROX_FRAMES_PER_SECOND 30

#include "fragment-shader.h"
#include "shader.h"
#include "vertex-shader.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>

struct alignas(16) UniformBlock
{
    float screen_dimensions[2]; // width height (pixels)
    float mouse_postion[2];     // mouse_x mouse_y (pixels)
    float iTime;                // time seconds
    int frame;                  // current frame
};

struct ApplicationContext
{
    SDL_GPUDevice* graphics_device;
    SDL_Window* window;
    SDL_GPUGraphicsPipeline* pipeline;

    std::filesystem::path fragment_shader_file;
    std::filesystem::file_time_type pipeline_last_regenerate_time;

    Shader* fragment_shader = nullptr;
    Shader* vertex_shader = nullptr;

    UniformBlock uniform = {0};
};

ApplicationContext context = {0};

bool InitializeDeviceAndWindow(const uint window_width,
                               const uint window_height)
{
    SDL_Init(SDL_INIT_VIDEO);

    context.graphics_device =
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);

    if (context.graphics_device == nullptr)
    {
        SDL_Log("CreateDevice failed.");
        return false;
    }

    context.window =
        SDL_CreateWindow("sr", window_width, window_height,
                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP);

    if (context.window == nullptr)
    {
        SDL_Log("CreateWindow failed: %s", SDL_GetError());
        return false;
    }

    if (!SDL_ClaimWindowForGPUDevice(context.graphics_device, context.window))
    {
        SDL_Log("ClaimWindow failed.");
        return false;
    }

    if (!SDL_SetGPUSwapchainParameters(
            context.graphics_device, context.window,
            SDL_GPUSwapchainComposition::
                SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR,
            SDL_GPUPresentMode::SDL_GPU_PRESENTMODE_VSYNC))
    {
        SDL_Log("Set GPUSwapchain failed.");
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
    colorTargetInfo.format =
        SDL_GetGPUSwapchainTextureFormat(device, context.window);

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

void UpdateUniformWindow(UniformBlock& in_out_uniforms)
{
    SDL_Window* window = context.window;

    int width;
    int height;
    SDL_GetWindowSize(window, &width, &height);

    in_out_uniforms.screen_dimensions[0] = width;
    in_out_uniforms.screen_dimensions[1] = height;
}
void UpdateUniformMouse(UniformBlock& in_out_uniforms)
{
    SDL_GetMouseState(&in_out_uniforms.mouse_postion[0],
                      &in_out_uniforms.mouse_postion[1]);
}

void UpdateUniforms(UniformBlock& in_out_uniforms)
{
    UpdateUniformWindow(in_out_uniforms);
    UpdateUniformMouse(in_out_uniforms);

    context.uniform.iTime += 1.0f / APPROX_FRAMES_PER_SECOND;
    context.uniform.frame++;
}

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

        UpdateUniforms(context.uniform);

        SDL_PushGPUFragmentUniformData(cmdbuf, 0, &context.uniform,
                                       sizeof(UniformBlock));

        SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);

    return true;
}

bool RegenerateRenderPipline(Shader* vertex, Shader* fragment)
{
    if (context.pipeline != nullptr)
    {
        SDL_ReleaseGPUGraphicsPipeline(context.graphics_device,
                                       context.pipeline);
    }

    context.pipeline = CreateGraphicsPipeline(
        context.graphics_device, vertex->Load(context.graphics_device),
        fragment->Load(context.graphics_device, true));

    context.pipeline_last_regenerate_time =
        last_write_time(context.fragment_shader_file);

    return true;
}

bool ParseArguments(int argc, char** argv, std::string& out_shader_file_path)
{
    if (argc != 2)
    {
        return false;
    }

    out_shader_file_path = argv[1];

    return true;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    std::string fragment_shader_file_path;
    if (!ParseArguments(argc, argv, fragment_shader_file_path))
    {
        SDL_Log("Parsed invalid arguments.");
        SDL_Log("Usage: sr [PATH TO GLSL FRAGMENT SHADER]. Aborting.");
        return SDL_APP_FAILURE;
    }

    if (!InitializeDeviceAndWindow(512, 512))
    {
        SDL_Log("Initialization failed. Aborting.");
        return SDL_AppResult::SDL_APP_FAILURE;
    }

    context.fragment_shader_file = fragment_shader_file_path;

    context.vertex_shader = new VertexShader();
    context.fragment_shader =
        new FragmentShader(context.fragment_shader_file.c_str());

    RegenerateRenderPipline(context.vertex_shader, context.fragment_shader);

    return SDL_AppResult::SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    SDL_Delay(1.0f / APPROX_FRAMES_PER_SECOND);

    if (exists(context.fragment_shader_file) &&
        last_write_time(context.fragment_shader_file) !=
            context.pipeline_last_regenerate_time)
    {
        RegenerateRenderPipline(context.vertex_shader, context.fragment_shader);
    }

    // If the fragment shader is invalid, pause until valid.
    if (context.fragment_shader == nullptr)
    {
        return SDL_APP_CONTINUE;
    }

    bool drawSuccess =
        Draw(context.graphics_device, context.window, context.pipeline);

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
    delete context.vertex_shader;
    delete context.fragment_shader;
    SDL_WaitForGPUIdle(context.graphics_device);
    SDL_ReleaseGPUGraphicsPipeline(context.graphics_device, context.pipeline);
    SDL_ReleaseWindowFromGPUDevice(context.graphics_device, context.window);
    SDL_DestroyWindow(context.window);
    SDL_DestroyGPUDevice(context.graphics_device);
}
