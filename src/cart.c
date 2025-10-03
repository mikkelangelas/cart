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

    new_emu->gb = create_gb("test.gb");

    return new_emu;
}

void destroy_emulator(Emulator *emu) {
    if (emu == NULL) return;

    SDL_DestroyTexture(emu->screen_texture);
    SDL_DestroyRenderer(emu->renderer);
    SDL_DestroyWindow(emu->window);
    SDL_Quit();
    
    destroy_gb(emu->gb);

    free(emu);
}

uint8_t cart_run() {
    Emulator *emulator = create_emulator();

    if (emulator == NULL) return 0;
    
    // Main event loop
    while (emulator->should_close == 0) {
        while (SDL_PollEvent(&emulator->event)) cart_handle_events(emulator);

        if (emulator->gb == NULL) continue;

        gb_step(emulator->gb);

        if (emulator->gb->frame_ready == 1) cart_render(emulator);
    }

    destroy_emulator(emulator);
    return 1;
}

void cart_handle_events(Emulator *emu) {
    if (emu->keys[SDL_SCANCODE_ESCAPE]) emu->should_close = 1;

    if (emu->gb == NULL) return;

    joypad_reset(&emu->gb->joypad);

    if (emu->keys[SDL_SCANCODE_Z]) joypad_press(&emu->gb->joypad, JOYPAD_BUTTON_B);
    if (emu->keys[SDL_SCANCODE_X]) joypad_press(&emu->gb->joypad, JOYPAD_BUTTON_A);
    if (emu->keys[SDL_SCANCODE_C]) joypad_press(&emu->gb->joypad, JOYPAD_BUTTON_START);
    if (emu->keys[SDL_SCANCODE_V]) joypad_press(&emu->gb->joypad, JOYPAD_BUTTON_SELECT);
    if (emu->keys[SDL_SCANCODE_RIGHT]) joypad_press(&emu->gb->joypad, JOYPAD_BUTTON_RIGHT);
    if (emu->keys[SDL_SCANCODE_LEFT]) joypad_press(&emu->gb->joypad, JOYPAD_BUTTON_LEFT);
    if (emu->keys[SDL_SCANCODE_UP]) joypad_press(&emu->gb->joypad, JOYPAD_BUTTON_UP);
    if (emu->keys[SDL_SCANCODE_DOWN]) joypad_press(&emu->gb->joypad, JOYPAD_BUTTON_DOWN);
}

void cart_render(Emulator *emu) {
    void *pixels = NULL;
    int pitch = 0;

    SDL_LockTexture(emu->screen_texture, NULL, &pixels, &pitch);

    Uint32 *pixels_dest = (Uint32*)pixels;

    const SDL_PixelFormatDetails *format=
        SDL_GetPixelFormatDetails(SDL_GetWindowPixelFormat(emu->window));

    for (uint16_t p = 0; p < (GB_SCREEN_W * GB_SCREEN_H); p++) {
        switch (emu->gb->framebuffer[p]) {
            case 0:
                pixels_dest[p] = SDL_MapRGB(format, NULL, 255, 255, 255); break;
            case 1:
                pixels_dest[p] = SDL_MapRGB(format, NULL, 170, 170, 170); break;
            case 2:
                pixels_dest[p] = SDL_MapRGB(format, NULL, 85, 85, 85); break;
            case 3:
                pixels_dest[p] = SDL_MapRGB(format, NULL, 0, 0, 0); break;
        }
    }

    emu->gb->frame_ready = 0;

    SDL_UnlockTexture(emu->screen_texture);

    SDL_SetRenderDrawColor(emu->renderer, 255, 255, 0, 255);
    SDL_RenderClear(emu->renderer);
    SDL_RenderTexture(emu->renderer, emu->screen_texture, NULL, NULL);
    SDL_RenderPresent(emu->renderer);
}
