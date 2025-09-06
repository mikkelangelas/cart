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
        .gameboy = gb
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
                    // TODO -> obj choosing
                    ppu->mode = PPU_MODE_PIXEL_DRAW;
                    ppu->current_dot = 0x00;
                }
                break;

            case PPU_MODE_PIXEL_DRAW:
                if (ppu->current_dot == PIXEL_DRAW_DOTS) {
                    // TODO -> drawing scanline
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
                    gameboy_interrupt(ppu->gameboy, INTERRUPT_VBLANK);

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

void ppu_draw_scanline(PPU *ppu) {

}

void ppu_draw_bg_line(PPU *ppu) {

}

void ppu_draw_wind_line(PPU *ppu) {

}

void ppu_draw_obj_line(PPU *ppu) {

}
