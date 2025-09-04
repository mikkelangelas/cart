#include "ppu.h"

void ppu_init(PPU *ppu, struct Gameboy *gb) {
    *ppu = (PPU){
        .mode = PPU_MODE_OAM_SCAN,
        .dot_counter = 0x00,
        .current_line = 0x00,
        .gameboy = gb
    };
}

void ppu_step(PPU *ppu, uint8_t cycles) {
    ppu->dot_counter += cycles * 4;

    switch (ppu->mode) {
        case PPU_MODE_OAM_SCAN: {
            break;
        }

        case PPU_MODE_PIXEL_DRAW: {
            break;
        }

        case PPU_MODE_HBLANK: {
            break;
        }

        case PPU_MODE_VBLANK: {
            break;
        }
    }
}
