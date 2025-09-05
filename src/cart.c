#include "cart.h"

#include <stdlib.h>
#include <stdio.h>

Emulator *create_emulator() {
    Emulator *new_emu = (Emulator*)malloc(sizeof(Emulator));

    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        fprintf(stderr, "create_emulator(): Failed to initialize SDL.\n");
        destroy_emulator(new_emu);
        return NULL;
    }

    new_emu->window = SDL_CreateWindow("cart", GB_SCREEN_W, GB_SCREEN_H, 0);
    
    if (new_emu->window == NULL) {
        fprintf(stderr, "create_emulator(): Failed to create a window.\n");
        destroy_emulator(new_emu);
        return NULL;
    }

    new_emu->renderer = SDL_CreateRenderer(new_emu->window, NULL);

    if (new_emu->renderer == NULL) {
        fprintf(stderr, "create_emulator(): Failed to create a renderer.\n");
        destroy_emulator(new_emu);
        return NULL;
    }

    new_emu->screen_texture = SDL_CreateTexture(
            new_emu->renderer,
            SDL_GetWindowPixelFormat(new_emu->window),
            SDL_TEXTUREACCESS_STREAMING,
            GB_SCREEN_W,
            GB_SCREEN_H
    );

    if (new_emu->screen_texture == NULL) {
        fprintf(stderr, "create_emulator(): Failed to create a screen texture.\n");
        destroy_emulator(new_emu);
        return NULL;
    }

    new_emu->keys = SDL_GetKeyboardState(NULL);

    new_emu->should_close = 0;

    new_emu->gameboy = NULL;

    return new_emu;
}

void destroy_emulator(Emulator *emu) {
    if (emu == NULL) return;

    SDL_DestroyTexture(emu->screen_texture);
    SDL_DestroyRenderer(emu->renderer);
    SDL_DestroyWindow(emu->window);
    SDL_Quit();
    
    destroy_gameboy(emu->gameboy);

    free(emu);
}

uint8_t cart_run() {
    Emulator *emulator = create_emulator();

    if (emulator == NULL) return 0;
    
    // Main event loop
    while (emulator->should_close == 0) {
        while (SDL_PollEvent(&emulator->event)) cart_handle_events(emulator);

        SDL_RenderClear(emulator->renderer);

        SDL_SetRenderDrawColor(emulator->renderer, 0x00, 0xFF, 0xFF, 0x00);
        SDL_RenderPresent(emulator->renderer);
    }

    destroy_emulator(emulator);
    return 1;
}

void cart_handle_events(Emulator *emu) {
    if (emu->keys[SDL_SCANCODE_ESCAPE]) emu->should_close = 1;
}
