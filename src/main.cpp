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
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>

using namespace std::filesystem;

SDL_GPUDevice* GraphicsDevice;
SDL_Window* Window;

SDL_GPUShader* FragmentShader = nullptr;
SDL_GPUShader* VertexShader = nullptr;
SDL_GPUGraphicsPipeline* Pipeline = nullptr;

path FragmentShaderFilePath;

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

float testTime = 0.0f;

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

        SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

        // Testing uniform (iTime)
        testTime += 0.01;
        SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);

    return true;
}

bool CompileGLSL(const char* sourceText, size_t sourceSize, const bool vertex,
                 char** output, size_t* outputSize)
{
    shaderc_compiler_t glslCompiler = shaderc_compiler_initialize();

    shaderc_compilation_result_t result = shaderc_compile_into_spv(
        glslCompiler, sourceText, sourceSize,
        vertex ? shaderc_glsl_vertex_shader : shaderc_glsl_fragment_shader,
        "default", GLSL_ENTRY_POINT, nullptr);

    shaderc_compiler_release(glslCompiler);

    const auto status = shaderc_result_get_compilation_status(result);

    if (status != shaderc_compilation_status_success)
    {
        const size_t errors = shaderc_result_get_num_errors(result);

        SDL_Log("Failed to compile GLSL code. Encountered %zd errors.", errors);
        SDL_Log("%s", shaderc_result_get_error_message(result));

        shaderc_result_release(result);
        return false;
    }

    const char* bytes = shaderc_result_get_bytes(result);
    const size_t size = shaderc_result_get_length(result);

    *output = (char*)malloc(size);
    memcpy(*output, bytes, size);

    shaderc_result_release(result);

    *outputSize = size;

    SDL_Log("Compiled shader of length : %zd", size);

    return true;
}

SDL_GPUShader* LoadShaderFromGLSL(SDL_GPUDevice* device,
                                  const char* shaderFilePath, const bool vertex)
{
    size_t fileSize;
    const char* file = (char*)SDL_LoadFile(shaderFilePath, &fileSize);

    char* shaderCode;
    size_t shaderCodeSize;

    const bool compileSuccess =
        CompileGLSL(file, fileSize, vertex, &shaderCode, &shaderCodeSize);

    SDL_free((void*)file);

    if (!compileSuccess)
    {
        return nullptr;
    }

    SDL_GPUShaderCreateInfo info = {0};
    info.code = (Uint8*)shaderCode;
    info.code_size = shaderCodeSize;
    info.entrypoint = GLSL_ENTRY_POINT;
    info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage =
        vertex ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
    free(shaderCode);

    return shader;
}

bool RegenerateRenderPipline(const char* vertexShaderSourcePath,
                             const char* fragmentShaderSourcePath)
{
    FragmentShader =
        LoadShaderFromGLSL(GraphicsDevice, fragmentShaderSourcePath, false);
    VertexShader =
        LoadShaderFromGLSL(GraphicsDevice, vertexShaderSourcePath, true);

    if (FragmentShader == nullptr || VertexShader == nullptr)
    {
        return false;
    }

    Pipeline =
        CreateGraphicsPipeline(GraphicsDevice, VertexShader, FragmentShader);

    SDL_ReleaseGPUShader(GraphicsDevice, VertexShader);
    SDL_ReleaseGPUShader(GraphicsDevice, FragmentShader);

    return true;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!InitializeDeviceAndWindow(256, 256))
    {
        SDL_Log("Initialization failed. Aborting.");
        return SDL_AppResult::SDL_APP_FAILURE;
    }

    FragmentShaderFilePath = path("shader.glsl");

    RegenerateRenderPipline("vertex.glsl", "shader.glsl");

    return SDL_AppResult::SDL_APP_CONTINUE;
}

file_time_type lastGenerateTime;

SDL_AppResult SDL_AppIterate(void* appstate)
{
    if (exists(FragmentShaderFilePath) &&
        last_write_time(FragmentShaderFilePath) != lastGenerateTime)
    {
        lastGenerateTime = last_write_time(FragmentShaderFilePath);
        RegenerateRenderPipline("vertex.glsl", FragmentShaderFilePath.c_str());
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
