#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event event;
bool Initialize();
void ClearAll();
int main(int argc, char *argv[]) {
  Initialize();

  ClearAll();
  return EXIT_SUCCESS;
}

bool Initialize() {
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialized. Error: %s\n", SDL_GetError());
    return false;
  }
  window = SDL_CreateWindow("Chip8 emulator", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
  if (!window) {
    printf("Can create window. Error: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

void ClearAll() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
