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
    for (uint8_t d = 0; d <= (cycles * DOTS_PER_CYCLE_DMG); d++) {
        ppu->current_dot++;

        switch (ppu->mode) {
            case PPU_MODE_OAM_SCAN:
                if (ppu->current_dot == OAM_SCAN_DOTS) {
                    ppu_scan_oam(ppu);
                    ppu->mode = PPU_MODE_PIXEL_DRAW;
                    ppu->current_dot = 0x00;
                }
                break;

            case PPU_MODE_PIXEL_DRAW:
                if (ppu->current_dot == PIXEL_DRAW_DOTS) {
                    ppu_draw_scanline(ppu);
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

void ppu_scan_oam(PPU *ppu) {
    uint8_t sprite_h = (get_bit(mmu_read(&ppu->gb->mmu, LCDC_ADDR), 2) == 0) ? 8 : 16;

    for (uint8_t o = 0; o < 40; o++) {
        uint8_t obj_y = mmu_read(&ppu->gb->mmu, OAM_ADDR + (o * 4));

        if (ppu->current_line - obj_y < sprite_h) {
            ppu->selected_objs[ppu->num_objs] = 0;
            ppu->num_objs++;
        }

        if (ppu->num_objs == 0) return;
    }
}

void ppu_draw_scanline(PPU *ppu) {
    ppu_draw_bg_line(ppu);
    ppu_draw_wind_line(ppu);
    ppu_draw_obj_line(ppu);
}

void ppu_draw_bg_line(PPU *ppu) {

}

void ppu_draw_wind_line(PPU *ppu) {

}

void ppu_draw_obj_line(PPU *ppu) {

}
