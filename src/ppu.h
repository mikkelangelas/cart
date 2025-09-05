#ifndef PPU_H
#define PPU_H

#include <stdint.h>

#define OAM_SCAN_DOTS 80
#define PIXEL_DRAW_DOTS 172
#define HBLANK_DOTS 204
#define VBLANK_DOTS 4560

#define DOTS_PER_LINE 456

#define VRAM_ADDR 0x8000
#define OAM_ADDR 0xFE00

#define LCDC_ADDR 0xFF40
#define STAT_ADDR 0xFF41
#define SCY_ADDR 0xFF42
#define SCX_ADDR 0xFF43
#define LY_ADDR 0xFF44
#define LYC_ADDR 0xFF45

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

void ppu_draw_scanline(PPU *ppu);

void ppu_draw_bg_line(PPU *ppu);

void ppu_draw_wind_line(PPU *ppu);

void ppu_draw_obj_line(PPU *ppu);

#endif
