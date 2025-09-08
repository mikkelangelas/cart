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

#define BGP_ADDR  0xFF47
#define OBP0_ADDR 0xFF48
#define OBP1_ADDR 0xFF49

#define WY_ADDR   0xFF4A
#define WX_ADDR   0xFF4B

// it's better to test a bit with an AND
#define LCDC_BG_WIND_ENABLE_MASK 0x01
#define LCDC_OBJ_ENABLE_MASK     0x02
#define LCDC_OBJ_SIZE_MASK       0x04
#define LCDC_BG_TILE_MAP_MASK    0x08
#define LCDC_BG_WIND_TILES_MASK  0x10
#define LCDC_WIND_ENABLE_MASK    0x20
#define LCDC_WIND_TILE_MAP_MASK  0x40
#define LCDC_LCD_PPU_ENABLE_MASK 0x80

#define STAT_LYC_INT_MASK   0x42
#define STAT_MODE0_INT_MASK 0x0B
#define STAT_MODE1_INT_MASK 0x13
#define STAT_MODE2_INT_MASK 0x23

#define STAT_MODE0_INT_CHECK 0x08
#define STAT_MODE1_INT_CHECK 0x11
#define STAT_MODE2_INT_CHECK 0x22

#define TILE_SIZE 8

#define OBJ_HEIGHT_SHORT 8
#define OBJ_HEIGHT_TALL 16

#define MAP_SIZE_TILES 32
#define MAP_SIZE_PIXELS 256

typedef enum PPUMode {
    PPU_MODE_HBLANK     = 0,
    PPU_MODE_VBLANK     = 1,
    PPU_MODE_OAM_SCAN   = 2,
    PPU_MODE_PIXEL_DRAW = 3
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

void ppu_scan_oam(PPU *ppu, uint8_t lcdc);

void ppu_draw_scanline(PPU *ppu, uint8_t lcdc);

void ppu_draw_bg_line(PPU *ppu, uint8_t lcdc);

void ppu_draw_wind_line(PPU *ppu, uint8_t lcdc);

void ppu_draw_obj_line(PPU *ppu, uint8_t lcdc);

void ppu_update_stat(PPU *ppu);

#endif
