#include "ppu.h"

#include "gameboy.h"
#include "util.h"

void ppu_init(PPU *ppu, struct Gameboy *gb) {
    *ppu = (PPU){
        .mode = PPU_MODE_OAM_SCAN,
        .current_dot = 0x00,
        .current_line = 0x00,
        .selected_objs = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        .num_objs = 0x00,
        .gb = gb
    };
}

void ppu_step(PPU *ppu, uint8_t cycles) {
    // Processing dot by dot should yield the most accurate
    // timing results, but does it really matter in
    // an emulator?
    uint8_t lcdc = mmu_read(&ppu->gb->mmu, LCDC_ADDR);

    if ((lcdc & LCDC_LCD_PPU_ENABLE_MASK) == 0x00) return;

    for (uint8_t d = 0; d <= (cycles * DOTS_PER_CYCLE_DMG); d++) {
        ppu->current_dot++;

        switch (ppu->mode) {
            case PPU_MODE_OAM_SCAN:
                if (ppu->current_dot == OAM_SCAN_DOTS) {
                    ppu_scan_oam(ppu, lcdc);
                    ppu->mode = PPU_MODE_PIXEL_DRAW;
                    ppu->current_dot = 0x00;
                }
                break;

            case PPU_MODE_PIXEL_DRAW:
                if (ppu->current_dot == PIXEL_DRAW_DOTS) {
                    ppu_draw_scanline(ppu, lcdc);
                    ppu->mode = PPU_MODE_HBLANK;
                    ppu->current_dot = 0x00;
                }
                break;

            case PPU_MODE_HBLANK:
                if (ppu->current_dot == HBLANK_DOTS) {
                   ppu->mode = (ppu->current_line == 143)
                       ? PPU_MODE_VBLANK
                       : PPU_MODE_OAM_SCAN;
                   ppu->current_dot = 0x00;
                   ppu->current_line++;
                }
                break;

            case PPU_MODE_VBLANK:
                if (ppu->current_dot == 1)
                    gameboy_interrupt(ppu->gb, INTERRUPT_VBLANK);

                if (ppu->current_dot % DOTS_PER_LINE == 0)
                    ppu->current_line++;

                if (ppu->current_dot == VBLANK_DOTS) {
                    ppu->mode = PPU_MODE_OAM_SCAN;
                    ppu->current_dot = 0x00;
                    ppu->current_line = 0x00;
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

}

void ppu_draw_wind_line(PPU *ppu, uint8_t lcdc) {

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
            gameboy_interrupt(ppu->gb, INTERRUPT_STAT);

    }

    mmu_write(&ppu->gb->mmu, STAT_ADDR);
}
