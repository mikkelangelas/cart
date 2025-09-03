#ifndef MMU_H
#define MMU_H

#include <stdint.h>

typedef struct MMU {
    struct Gameboy *gameboy;
} MMU;

uint8_t read_byte(MMU *mmu, uint16_t addr);
uint16_t read_word(MMU *mmu, uint16_t addr);

void write_byte(MMU *mmu, uint16_t addr, uint8_t val);
void write_word(MMU *mmu, uint16_t addr, uint16_t val);

#endif
