#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <ostream>

#define SHADER_FILE "./shader.glsl"
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

// https://hamdy-elzanqali.medium.com/let-there-be-triangles-sdl-gpu-edition-bd82cf2ef615

SDL_Window* window;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
  window =
      SDL_CreateWindow(const char* title, int w, int h, SDL_WindowFlags flags)
}

/*
int main()
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Window* window =
      SDL_CreateWindow("sr", WINDOW_WIDTH, WINDOW_HEIGHT,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_ALWAYS_ON_TOP);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // Setup GLEW
  glewExperimental = true;
  GLenum err = glewInit();
  if (err != GLEW_OK)
  {
    cerr << "Glew init failed (" << glewGetErrorString(err) << ")." << endl;
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  cout << "GLEW init complete." << endl;

  GLuint

      SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
*/
