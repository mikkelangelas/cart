#ifndef APU_H
#define APU_H

#include <stdint.h>

#define NR10_ADDR_REL 0x0010
#define NR11_ADDR_REL 0x0011
#define NR12_ADDR_REL 0x0012
#define NR13_ADDR_REL 0x0013
#define NR14_ADDR_REL 0x0014

#define NR21_ADDR_REL 0x0016
#define NR22_ADDR_REL 0x0017
#define NR23_ADDR_REL 0x0018
#define NR24_ADDR_REL 0x0019

#define NR30_ADDR_REL 0x001A
#define NR31_ADDR_REL 0x001B
#define NR32_ADDR_REL 0x001C
#define NR33_ADDR_REL 0x001D
#define NR34_ADDR_REL 0x001E

#define NR41_ADDR_REL 0x0020
#define NR42_ADDR_REL 0x0021
#define NR43_ADDR_REL 0x0022
#define NR44_ADDR_REL 0x0023

#define NR50_ADDR_REL 0x0024
#define NR51_ADDR_REL 0x0025
#define NR52_ADDR_REL 0x0026

#define WAVE_ADDR_REL 0x0030

typedef struct APU {
    struct GB *gb;
} APU;

void apu_init(APU *apu, struct GB *gb);

void apu_step(APU *apu, uint8_t cycles);

#endif