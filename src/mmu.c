#include "mmu.h"

#include "gb.h"
#include "bootrom.h"

#include <stdio.h>

void mmu_init(MMU *mmu, struct GB *gb) {
    *mmu = (MMU){
        .bootrom_mapped = 1,
        .gb = gb
    };
}

uint8_t mmu_read(MMU *mmu, uint16_t addr) {
    // TODO -> optimize this
    uint8_t val = 0xFF;

    if (0x0000 <= addr && addr <= 0x7FFF) {
        if (mmu->bootrom_mapped == 1 && addr < 0x0100)
            val = DMG_BOOTROM[addr];
        else
            val = cartridge_read(mmu->gb->cartridge, addr);
    }
    else if (0x8000 <= addr && addr <= 0x9FFF) val = mmu->gb->vram[addr - 0x8000];
    else if (0xA000 <= addr && addr <= 0xBFFF) val = cartridge_read(mmu->gb->cartridge, addr);
    else if (0xC000 <= addr && addr <= 0xDFFF) val = mmu->gb->wram[addr - 0xC000];
    else if (0xE000 <= addr && addr <= 0xFDFF) val = mmu->gb->wram[addr - 0xE000];
    else if (0xFE00 <= addr && addr <= 0xFE9F) val = mmu->gb->oam[addr - 0xFE00];
    else if (0xFEA0 <= addr && addr <= 0xFEFF) {
        // prohibited, used to emulate OAM corruption bug
        return 0xFF;
    }
    else if (0xFF00 <= addr && addr <= 0xFF7F) val = mmu->gb->io_registers[addr - 0xFF00];
    else if (0xFF80 <= addr && addr <= 0xFFFE) val = mmu->gb->hram[addr - 0xFF90];
    else if (addr == 0xFFFF) val = mmu->gb->ie;

    return val;
}

void mmu_write(MMU *mmu, uint16_t addr, uint8_t val) {
    switch (addr >> 12) {
        case 0x08:
        case 0x09:
            mmu->gb->vram[addr - 0x8000] = val; break;

        case 0x0A:
        case 0x0B:
            cartridge_write(mmu->gb->cartridge, addr, val); break;

        case 0x0C:
        case 0x0D:
            mmu->gb->wram[addr - 0xC000] = val; break;

        case 0x0E:
        case 0x0F:
            if (addr <= 0xFDFF) mmu->gb->wram[addr - 0xE000] = val;
            else if (addr <= 0xFE9F) mmu->gb->oam[addr - 0xFE00] = val;
            else if (addr <= 0xFEFF) return;
            else if (addr <= 0xFF7F) mmu->gb->io_registers[addr - 0xFF00] = val;
            else if (addr <= 0xFFFE) mmu->gb->hram[addr - 0xFF80] = val;
            else mmu->gb->ie = val;
    }
}

void mmu_dma_transfer(MMU *mmu, uint8_t start) {
    if (start > 0xDF) return;

    uint16_t src = start << 8;

    for (uint8_t b = 0; b < OAM_SIZE; b++)
        mmu_write(mmu, OAM_ADDR + b, mmu_read(mmu, src + b));
}
