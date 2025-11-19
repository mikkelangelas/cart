#include "apu.h"

#include "gb.h"

void apu_init(APU *apu, struct GB *gb) {
    apu->gb = gb;
}

void apu_step(APU *apu, uint8_t cycles) {
    
}