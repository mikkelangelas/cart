#include "ppu.h"
#include "gb.h"
#include "util.h"

static inline void horizontal_rectrace(PPU *ppu) {
    ppu->gb->io[LY_ADDR_RELATIVE] = ++ppu->current_line;
}

static inline void vertical_retrace(PPU *ppu) {
    ppu->gb->io[LY_ADDR_RELATIVE] = ppu->current_line= 0;
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

static inline uint8_t get_color_index(uint16_t row_data, uint8_t tile_x) {
    return (row_data >> ((7 - tile_x) * 2)) & 0x03;
}

static uint8_t fetch_palette_color(PPU *ppu, uint16_t pal_addr, uint8_t color_idx) {
    uint8_t palette = ppu->gb->io[pal_addr];

    return (palette >> (color_idx * 2)) & 0x03;
}

static inline void draw_pixel(PPU *ppu, uint8_t x, uint8_t y, uint8_t color) {
    ppu->gb->framebuffer[y * GB_SCREEN_W + x] = color;
}

static inline uint8_t get_pixel(PPU *ppu, uint8_t x, uint8_t y) {
    return ppu->gb->framebuffer[y * GB_SCREEN_W + x];
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
    uint8_t lcdc = ppu->gb->io[LCDC_ADDR_RELATIVE];

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
                    horizontal_rectrace(ppu);
                }
                break;

            case PPU_MODE_VBLANK:
                if (ppu->current_dot == 1) {
                    gb_interrupt(ppu->gb, INTERRUPT_VBLANK);
                    ppu->gb->frame_ready = 1;
                }

                if (ppu->current_dot % DOTS_PER_LINE == 0)
                    horizontal_rectrace(ppu);


                if (ppu->current_dot == VBLANK_DOTS) {
                    ppu->mode = PPU_MODE_OAM_SCAN;
                    ppu->current_dot = 0;
                    vertical_retrace(ppu);
                }
                break;
        }
    }
}

void ppu_scan_oam(PPU *ppu, uint8_t lcdc) {
    ppu->num_objs = 0;

    uint8_t sprite_h = (lcdc & LCDC_OBJ_SIZE_MASK)
        ? OBJ_HEIGHT_TALL
        : OBJ_HEIGHT_SHORT;

    for (uint8_t o = 0; o < 40; o++) {
        uint8_t obj_y = mmu_read(&ppu->gb->mmu, OAM_BASE_ADDR + (o * 4));

        if (obj_y > 16 - sprite_h && ppu->current_line + 16 >= obj_y && ppu->current_line + 16 < obj_y + sprite_h)
            ppu->selected_objs[ppu->num_objs++] = o;

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

    uint8_t scx = ppu->gb->io[SCX_ADDR_RELATIVE];

    uint8_t scrolled_y = ppu->gb->io[SCY_ADDR_RELATIVE] + ppu->current_line;

    // no need to use % 256 for wrapping scrolled_y around 
    // because it's 8-bit so it will naturally happen
    // in case an overflow occurs
    uint8_t map_y = scrolled_y / TILE_SIZE;
    uint8_t tile_y = scrolled_y % TILE_SIZE;

    uint8_t last_tile_idx = 0;
    uint16_t tile_data = 0;

    for (uint8_t screen_x = 0; screen_x < GB_SCREEN_W; screen_x++) {
        uint8_t scrolled_x = screen_x + scx; 
        uint8_t map_x = scrolled_x / TILE_SIZE;
        uint8_t tile_x = scrolled_x % TILE_SIZE;

        uint8_t tile_idx = mmu_read(&ppu->gb->mmu, map_addr + (map_y * MAP_SIZE_TILES) + map_x);

        // prevents refetching tile data for every pixel
        if (tile_idx != last_tile_idx || screen_x == 0) {
            last_tile_idx = tile_idx;
            tile_data = fetch_tile_row_data(ppu, tiles_addr, tile_idx, tile_y);
        }

        uint8_t color = fetch_palette_color(ppu, BGP_ADDR_RELATIVE, get_color_index(tile_data, tile_x));

        draw_pixel(ppu, screen_x, ppu->current_line, color);
    }
}

void ppu_draw_wind_line(PPU *ppu, uint8_t lcdc) {
    uint16_t tiles_addr = (lcdc & LCDC_BG_WIND_TILES_MASK) ? 0x8000 : 0x8800;
    uint16_t map_addr = (lcdc & LCDC_WIND_TILE_MAP_MASK) ? 0x9C00 : 0x9800;

    uint8_t wx = ppu->gb->io[WX_ADDR_RELATIVE];
    uint8_t wy = ppu->gb->io[WY_ADDR_RELATIVE];

    uint8_t wind_y = ppu->current_line - wy;

    if (wx >= GB_SCREEN_W + 7 || wy >= GB_SCREEN_H) return;

    uint8_t map_y = wind_y / TILE_SIZE;
    uint8_t tile_y = wind_y % TILE_SIZE;

    uint8_t last_tile_idx = 0;
    uint16_t tile_data = 0;

    for (uint8_t wind_x = 0; wind_x < GB_SCREEN_W; wind_x++) {
        if (wx + wind_x < 7) continue;

        uint8_t screen_x = wx + wind_x - 7;

        uint8_t map_x = wind_x / TILE_SIZE;
        uint8_t tile_x = wind_x % TILE_SIZE;

        uint8_t tile_idx = mmu_read(&ppu->gb->mmu, map_addr + (map_y * MAP_SIZE_TILES) + map_x);

        // prevents refetching tile data for every pixel
        if (tile_idx != last_tile_idx || wind_x == 0) {
            last_tile_idx = tile_idx;
            tile_data = fetch_tile_row_data(ppu, tiles_addr, tile_idx, tile_y);
        }

        uint8_t color = fetch_palette_color(ppu, BGP_ADDR_RELATIVE, get_color_index(tile_data, tile_x));

        draw_pixel(ppu, screen_x, ppu->current_line, color);
    }
}

void ppu_draw_obj_line(PPU *ppu, uint8_t lcdc) {
    for (uint8_t o = 0; o < ppu->num_objs; o++) {
        uint16_t obj_loc = OAM_BASE_ADDR + (ppu->selected_objs[o] * 4);

        uint8_t obj_x = mmu_read(&ppu->gb->mmu, obj_loc + OAM_X_OFFSET);

        // object beyond the screenspace
        if (obj_x == 0 || obj_x >= 168)
            continue;

        uint8_t obj_attr = mmu_read(&ppu->gb->mmu, obj_loc + OAM_ATTR_OFFSET);

        // checking the object's attributes
        uint8_t flipped_x = obj_attr & OAM_ATTR_X_FLIP_MASK;
        uint8_t flipped_y = obj_attr & OAM_ATTR_Y_FLIP_MASK;
        uint8_t low_priority = obj_attr & OAM_ATTR_PRIORITY_MASK;
        uint16_t pal_addr = (obj_attr & OAM_ATTR_PALETTE_MASK) ? OBP1_ADDR_RELATIVE : OBP0_ADDR_RELATIVE;

        uint8_t obj_y = mmu_read(&ppu->gb->mmu, obj_loc) - 16;
        uint8_t tile_y = (ppu->current_line - obj_y) % 8;

        if (flipped_y) tile_y = 7 - tile_y;

        uint8_t tile_idx = mmu_read(&ppu->gb->mmu, obj_loc + OAM_TILE_OFFSET);

        if (ppu->current_line - obj_y >= 8)
            tile_idx++;

        uint16_t tile_data = fetch_tile_row_data(ppu, 0x8000, tile_idx, tile_y);

        for (uint8_t p = 0; p < 8; p++) {
            if (obj_x + p < 8) continue; // pixel outside the screen
            
            uint8_t color_idx = get_color_index(tile_data, (flipped_x) ? 7 - p : p);

            if (color_idx == 0) continue; // pixel is transparent
            
            uint8_t screen_x = obj_x - 8 + p;

            if (low_priority && fetch_palette_color(ppu, BGP_ADDR_RELATIVE, 0) != get_pixel(ppu, screen_x, ppu->current_line))
                continue; // object has low priority (bg and window colors indices 1-3 are drawn over)

            uint8_t color = fetch_palette_color(ppu, pal_addr, color_idx);

            draw_pixel(ppu, screen_x, ppu->current_line, color);
        }
    }
}

void ppu_update_stat(PPU *ppu) {
    uint8_t stat = ppu->gb->io[STAT_ADDR_RELATIVE];

    set_bit(&stat, 2, ppu->current_line == ppu->gb->io[LYC_ADDR_RELATIVE]);

    if (ppu->current_dot == 1) {
        stat = (stat & 0xFC) | ppu->mode;

        if (stat & STAT_LYC_INT_MASK ||
            (stat & STAT_MODE0_INT_MASK) == STAT_MODE0_INT_CHECK ||
            (stat & STAT_MODE1_INT_MASK) == STAT_MODE1_INT_CHECK ||
            (stat & STAT_MODE2_INT_MASK) == STAT_MODE2_INT_CHECK)
            gb_interrupt(ppu->gb, INTERRUPT_STAT);

    }

    ppu->gb->io[STAT_ADDR_RELATIVE] = stat;
}
