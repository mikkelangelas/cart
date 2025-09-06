#ifndef CART_H
#define CART_H

#include <stdint.h>
#include <SDL3/SDL.h>

#include "gameboy.h"

typedef struct Emulator {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen_texture;
    SDL_Event event;
    const bool *keys;

    uint8_t should_close;

    Gameboy *gb;
} Emulator;

Emulator *create_emulator();

void destroy_emulator(Emulator *emu);

uint8_t cart_run();

void cart_handle_events(Emulator *emu);

#endif
