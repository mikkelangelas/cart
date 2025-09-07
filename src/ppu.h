#ifndef PPU_H
#define PPU_H

#include <stdint.h>

#define OAM_SCAN_DOTS     80
#define PIXEL_DRAW_DOTS  172
#define HBLANK_DOTS      204
#define VBLANK_DOTS     4560

#define DOTS_PER_LINE    456
#define DOTS_PER_CYCLE_DMG 4

#define VRAM_ADDR 0x8000
#define OAM_ADDR  0xFE00

#define LCDC_ADDR 0xFF40
#define STAT_ADDR 0xFF41
#define SCY_ADDR  0xFF42
#define SCX_ADDR  0xFF43
#define LY_ADDR   0xFF44
#define LYC_ADDR  0xFF45

#define LCDC_BG_WIND_ENABLE_BIT 0
#define LCDC_OBJ_ENABLE_BIT     1
#define LCDC_OBJ_SIZE_BIT       2
#define LCDC_BG_TILE_MAP_BIT    3
#define LCDC_BG_WIND_TILES_BIT  4
#define LCDC_WIND_ENABLE_BIT    5
#define LCDC_WIND_TILE_MAP_BIT  6
#define LCDC_LCD_PPU_ENABLE_BIT 7

typedef enum PPUMode {
    PPU_MODE_OAM_SCAN,
    PPU_MODE_PIXEL_DRAW,
    PPU_MODE_HBLANK,
    PPU_MODE_VBLANK
} PPUMode;

typedef struct PPU {
    PPUMode mode;

    uint16_t current_dot;
    uint8_t current_line;

    uint8_t selected_objs[10];
    uint8_t num_objs;

    struct Gameboy *gb;
} PPU;

void ppu_init(PPU *ppu, struct Gameboy *gb);

void ppu_step(PPU *ppu, uint8_t cycles);

void ppu_scan_oam(PPU *ppu);

void ppu_draw_scanline(PPU *ppu);

void ppu_draw_bg_line(PPU *ppu);

void ppu_draw_wind_line(PPU *ppu);

void ppu_draw_obj_line(PPU *ppu);

#endif
