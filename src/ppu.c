#include "ppu.h"

void ppu_init(PPU *ppu, struct Gameboy *gb) {
    *ppu = (PPU){
        .dot_counter = 0x00,
        .current_line = 0x00,
        .gameboy = gb
    };
}

void ppu_step(PPU *ppu, uint8_t cycles) {

}
