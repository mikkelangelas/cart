#include "ppu.h"
#include "gb.h"
#include "util.h"
#include <stdio.h>

static inline void next_scanline(PPU *ppu) {
    mmu_write(&ppu->gb->mmu, LY_ADDR, ++ppu->current_line);
}

static uint16_t fetch_tile_row_data(PPU *ppu, uint16_t addr, uint8_t idx, uint8_t y) {
    if (addr == 0x8800) idx -= 128;
    
    uint16_t row_addr = addr + ((uint16_t)idx * 16) + ((uint16_t)y * 2);

    uint8_t row_lo = mmu_read(&ppu->gb->mmu, row_addr);
    uint8_t row_hi = mmu_read(&ppu->gb->mmu, row_addr + 1);

    uint16_t row_data = 0x0000;

    uint8_t mask = 0x01;

    for (uint8_t b = 0; b < 8; b++) {
        uint16_t lo_tmp = (uint16_t)(row_lo & mask) << b;
        uint16_t hi_tmp = (uint16_t)(row_hi & mask) << (b + 1);

        mask <<= 1;

        row_data |= lo_tmp;
        row_data |= hi_tmp;
    }

    return row_data;
}

static uint8_t fetch_pixel_color(PPU *ppu, uint16_t pal_addr, uint16_t row_data, uint8_t x) {
    uint8_t palette = mmu_read(&ppu->gb->mmu, pal_addr);

    uint8_t col_idx = (row_data >> ((7 - x) * 2)) & 0x03;

    return (palette >> (col_idx * 2)) & 0x03;
}

void ppu_init(PPU *ppu, struct GB *gb) {
    *ppu = (PPU){
        .mode = PPU_MODE_OAM_SCAN,
        .current_dot = 0,
        .current_line = 0,
        .selected_objs = {0,0,0,0,0,0,0,0,0,0},
        .num_objs = 0,
        .gb = gb
    };
}

void ppu_step(PPU *ppu, uint8_t cycles) {
    // Processing dot by dot should yield the most accurate
    // timing results, but does it really matter in
    // an emulator?
    uint8_t lcdc = mmu_read(&ppu->gb->mmu, LCDC_ADDR);

    if ((lcdc & LCDC_LCD_PPU_ENABLE_MASK) == 0) return;

    for (uint8_t d = 0; d <= (cycles * DOTS_PER_CYCLE_DMG); d++) {
        ppu->current_dot++;

        ppu_update_stat(ppu);

        switch (ppu->mode) {
            case PPU_MODE_OAM_SCAN:
                if (ppu->current_dot == OAM_SCAN_DOTS) {
                    ppu_scan_oam(ppu, lcdc);
                    ppu->mode = PPU_MODE_PIXEL_DRAW;
                    ppu->current_dot = 0;
                }
                break;

            case PPU_MODE_PIXEL_DRAW:
                if (ppu->current_dot == PIXEL_DRAW_DOTS) {
                    ppu_draw_scanline(ppu, lcdc);
                    ppu->mode = PPU_MODE_HBLANK;
                    ppu->current_dot = 0;
                }
                break;

            case PPU_MODE_HBLANK:
                if (ppu->current_dot == HBLANK_DOTS) {
                    ppu->mode = (ppu->current_line == 143)
                        ? PPU_MODE_VBLANK
                        : PPU_MODE_OAM_SCAN;
                    ppu->current_dot = 0;
                    next_scanline(ppu);
                }
                break;

            case PPU_MODE_VBLANK:
                if (ppu->current_dot == 1) {
                    gb_interrupt(ppu->gb, INTERRUPT_VBLANK);
                    ppu->gb->frame_ready = 1;
                }

                if (ppu->current_dot == VBLANK_DOTS) {
                    ppu->mode = PPU_MODE_OAM_SCAN;
                    ppu->current_dot = 0;
                    ppu->current_line = 0;
                    mmu_write(&ppu->gb->mmu, LY_ADDR, 0);
                }
                break;
        }
    }
}

void ppu_scan_oam(PPU *ppu, uint8_t lcdc) {
    uint8_t sprite_h = (lcdc & LCDC_OBJ_SIZE_MASK)
        ? OBJ_HEIGHT_TALL
        : OBJ_HEIGHT_SHORT;

    for (uint8_t o = 0; o < 40; o++) {
        uint8_t obj_y = mmu_read(&ppu->gb->mmu, OAM_ADDR + (o * 4));

        if (ppu->current_line - obj_y < sprite_h) {
            ppu->selected_objs[ppu->num_objs] = o;
            ppu->num_objs++;
        }

        if (ppu->num_objs == 10) return;
    }
}

void ppu_draw_scanline(PPU *ppu, uint8_t lcdc) {
    if (lcdc & LCDC_BG_WIND_ENABLE_MASK) {
        ppu_draw_bg_line(ppu, lcdc);

        if (lcdc & LCDC_WIND_ENABLE_MASK) ppu_draw_wind_line(ppu, lcdc);
    }

    if (lcdc & LCDC_OBJ_ENABLE_MASK) ppu_draw_obj_line(ppu, lcdc);
}

void ppu_draw_bg_line(PPU *ppu, uint8_t lcdc) {
    uint16_t tiles_addr = (lcdc & LCDC_BG_WIND_TILES_MASK) ? 0x8000 : 0x8800;
    uint16_t map_addr = (lcdc & LCDC_BG_TILE_MAP_MASK) ? 0x9C00 : 0x9800;

    uint8_t scx = mmu_read(&ppu->gb->mmu, SCX_ADDR);

    uint8_t scrolled_y = mmu_read(&ppu->gb->mmu, SCY_ADDR) + ppu->current_line;

    // no need to use % 256 for wrapping scrolled_y around 
    // because it's 8-bit so it will naturally happen
    // in case an overflow occurs
    uint8_t map_y = scrolled_y / TILE_SIZE; 
    uint8_t tile_y = scrolled_y % TILE_SIZE;

    uint8_t last_tile_idx = 0;
    uint16_t tile_data = 0;

    for (uint8_t screen_x = 0; screen_x < GB_SCREEN_W; screen_x++) {
        uint8_t scrolled_x = scx + screen_x; 
        uint8_t map_x = scrolled_x / TILE_SIZE;
        uint8_t tile_x = scrolled_x % TILE_SIZE;

        uint8_t tile_idx = mmu_read(&ppu->gb->mmu, map_addr + (map_y * MAP_SIZE_TILES) + map_x);

        // prevents refetching tile data for every pixel
        if (tile_idx != last_tile_idx || screen_x == 0) {
            last_tile_idx = tile_idx;
            tile_data = fetch_tile_row_data(ppu, tiles_addr, tile_idx, tile_y);
        }

        uint8_t color = fetch_pixel_color(ppu, BGP_ADDR, tile_data, tile_x);
        //printf("sy: %d sx: %d my: %d mx: %d ty: %d tx: %d tidx: %d color: %d\n", scrolled_y, scrolled_x, map_y, map_x, tile_y, tile_x, tile_idx, color);

        ppu->gb->framebuffer[ppu->current_line * GB_SCREEN_W + screen_x] = color;
    }
}

void ppu_draw_wind_line(PPU *ppu, uint8_t lcdc) {
    uint16_t tiles_addr = (lcdc & LCDC_BG_WIND_TILES_MASK) ? 0x8000 : 0x8800;
    uint16_t map_addr = (lcdc & LCDC_WIND_TILE_MAP_MASK) ? 0x9C00 : 0x9800;

    uint8_t wx = mmu_read(&ppu->gb->mmu, WX_ADDR);
    uint8_t wind_y = mmu_read(&ppu->gb->mmu, WY_ADDR) + ppu->current_line;

    if (wx >= GB_SCREEN_H + 7 || wind_y >= GB_SCREEN_W) return;

    uint8_t map_y = wind_y / TILE_SIZE;
    uint8_t tile_y = wind_y % TILE_SIZE;

    uint8_t last_tile_idx = 0;
    uint16_t tile_data = 0;

    for (uint8_t wind_x = wx; wind_x < GB_SCREEN_W; wind_x++) {
        if (wind_x < 7) continue;

        uint8_t map_x = wind_x / TILE_SIZE;
        uint8_t tile_x = wind_x % TILE_SIZE;

        uint8_t tile_idx = mmu_read(&ppu->gb->mmu, map_addr + (map_y * MAP_SIZE_TILES) + map_x);

        // prevents refetching tile data for every pixel
        if (tile_idx != last_tile_idx || wind_x == 0) {
            last_tile_idx = tile_idx;
            tile_data = fetch_tile_row_data(ppu, tiles_addr, tile_idx, tile_y);
        }

        uint8_t color = fetch_pixel_color(ppu, BGP_ADDR, tile_data, tile_x);

        ppu->gb->framebuffer[ppu->current_line * GB_SCREEN_W + (wind_x - 7)] = color;
    }
}

void ppu_draw_obj_line(PPU *ppu, uint8_t lcdc) {

}

void ppu_update_stat(PPU *ppu) {
    uint8_t stat = mmu_read(&ppu->gb->mmu, STAT_ADDR);

    set_bit(&stat, 2, ppu->current_line == mmu_read(&ppu->gb->mmu, LYC_ADDR));

    if (ppu->current_dot == 1) {
        stat = (stat & 0xFC) | ppu->mode;

        if (stat & STAT_LYC_INT_MASK ||
            (stat & STAT_MODE0_INT_MASK) == STAT_MODE0_INT_CHECK ||
            (stat & STAT_MODE1_INT_MASK) == STAT_MODE1_INT_CHECK ||
            (stat & STAT_MODE2_INT_MASK) == STAT_MODE2_INT_CHECK)
            gb_interrupt(ppu->gb, INTERRUPT_STAT);

    }

    mmu_write(&ppu->gb->mmu, STAT_ADDR, stat);
}
