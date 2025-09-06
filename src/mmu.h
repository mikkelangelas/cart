#ifndef MMU_H
#define MMU_H

#include <stdint.h>

typedef struct MMU {
    struct Gameboy *gb;
} MMU;

void mmu_init(MMU *mmu, struct Gameboy *gb);

uint8_t mmu_read(MMU *mmu, uint16_t addr);
void mmu_write(MMU *mmu, uint16_t addr, uint8_t val);

#endif
