#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#define BANK_ADDR 0xFF50

typedef struct MMU {
    uint8_t bootrom_mapped;
    struct GB *gb;
} MMU;

void mmu_init(MMU *mmu, struct GB *gb);

uint8_t mmu_read(MMU *mmu, uint16_t addr);
void mmu_write(MMU *mmu, uint16_t addr, uint8_t val);

void mmu_dma_transfer(MMU *mmu, uint8_t start);

#endif
