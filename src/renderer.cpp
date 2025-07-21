#include <SDL3/SDL_init.h>
#include <renderer.h>

Renderer::Renderer(int width, int height) { Initialize(width, height); }

void Renderer::Initialize(int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO);

    Device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);

    if (Device == nullptr)
    {
        SDL_Log("CreateDevice failed.");
        return;
    }

    Window = SDL_CreateWindow("Display", width, height,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP);
    if (Window == nullptr)
    {
        SDL_Log("CreateWindow failed: %s", SDL_GetError());
        return;
    }

    if (!SDL_ClaimWindowForGPUDevice(Device, Window))
    {
        SDL_Log("ClaimWindow failed.");
        return;
    }
}
