// ! Not used yet !

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

// Responsible for GPU and Window integration
class Renderer
{
    /************************************************************************************************/
    /** Renderer */

protected:

    SDL_GPUDevice* Device;

    SDL_Window* Window;

    int windowHeight;

    int windowWidth;

public:

    Renderer(int width, int height);

protected:

    void Initialize(int width, int height);
};
