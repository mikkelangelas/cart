#ifndef PPU_H
#define PPU_H

#include <stdint.h>

#define OAM_SCAN_DOTS 80
#define PIXEL_DRAW_DOTS 172
#define HBLANK_DOTS 204
#define VBLANK_DOTS 4560

#define DOTS_PER_LINE 456

typedef enum PPUMode {
    PPU_MODE_OAM_SCAN,
    PPU_MODE_PIXEL_DRAW,
    PPU_MODE_HBLANK,
    PPU_MODE_VBLANK
} PPUMode;

typedef struct PPU {
    PPUMode mode;

    uint16_t dot_counter;
    uint8_t current_line;

    struct Gameboy *gameboy;
} PPU;

void ppu_init(PPU *ppu, struct Gameboy *gb);

void ppu_step(PPU *ppu, uint8_t cycles);

#endif
