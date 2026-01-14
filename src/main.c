#include "chip8.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define WHITE 1
#define BLACK 0
#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32

/// SDL pointer
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event sdl_event;
/// status flag
bool closeWindow = false;
// init the chip
chip8_t chip;

// prototype function
bool Initialize();
void ClearAll();
void UpdateScreen(chip8_t *chip);

/// main character ///
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Not given rom path\nUsage: ./chip8_emu <rom_file>\n");
    return EXIT_FAILURE;
  }
  char *rom_path = argv[1];
  Initialize();
  if (!load_rom(&chip, rom_path))
    return EXIT_FAILURE;
  // Init cpu and timer tick
  uint32_t last_cpu_tick = 0;
  uint32_t last_timer_tick = 0;
  //
  const uint32_t CPU_INTERVAL = 1000 / 500;
  const uint32_t TIMER_INTERVAL = 1000 / 60;
  while (!closeWindow) {
    while (SDL_PollEvent(&sdl_event)) {
      if (sdl_event.type == SDL_QUIT)
        closeWindow = true;
    }
    uint32_t now = SDL_GetTicks();
    while (now - last_cpu_tick >= CPU_INTERVAL) {
      decode_and_execute(&chip, fetch(&chip));
      last_cpu_tick += CPU_INTERVAL;
    }
    while (now - last_timer_tick >= TIMER_INTERVAL) {
      if (chip.delay_timer > 0)
        chip.delay_timer--;
      if (chip.sound_timer > 0)
        chip.sound_timer--;
      last_timer_tick += TIMER_INTERVAL;
    }
    if (chip.drawFlag == 1) {
      SDL_SetRenderDrawColor(renderer, 71, 71, 72, 255); /// dark color
      SDL_RenderClear(renderer);
      UpdateScreen(&chip);
      SDL_RenderPresent(renderer);
      chip.drawFlag = 0;
    }
    SDL_Delay(1);
  }

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
  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    printf("Can't create renderer. Error: %s\n", SDL_GetError());
    return false;
  }
  chip8_init(&chip);
  SDL_RenderSetScale(renderer, 10, 10);

  return true;
}

void ClearAll() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void UpdateScreen(chip8_t *chip) {
  SDL_SetRenderDrawColor(renderer, 178, 255, 38, 255);
  for (int i = 0; i < CHIP8_WIDTH; i++)
    for (int j = 0; j < CHIP8_HEIGHT; j++) {
      if (chip->display[i + 64 * j] == WHITE)
        SDL_RenderDrawPoint(renderer, i, j);
    }
}
