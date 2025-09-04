#ifndef PPU_H
#define PPU_H

#include <stdint.h>

typedef struct PPU {
    uint8_t dot_counter;
    uint8_t current_line;

    struct Gameboy *gameboy;
} PPU;

void ppu_init(PPU *ppu, struct Gameboy *gb);

void ppu_step(PPU *ppu, uint8_t cycles);

#endif
