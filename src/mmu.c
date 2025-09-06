#include "mmu.h"

#include "gameboy.h"

void mmu_init(MMU *mmu, struct Gameboy *gb) {
    mmu->gb = gb;
}

uint8_t mmu_read(MMU *mmu, uint16_t addr) {
    uint8_t val = 0xFF;

    if (0x0000 <= addr && addr <= 0x7FFF) {
        // cartridge ROM 
    }
    else if (0x8000 <= addr && addr <= 0x9FFF) val = mmu->gb->vram[addr - 0x8000];
    else if (0xA000 <= addr && addr <= 0xBFFF) {
        // cartridge RAM
    }
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
    if (0x0000 <= addr && addr <= 0x7FFF) {
        // cartridge ROM 
    }
    else if (0x8000 <= addr && addr <= 0x9FFF) mmu->gb->vram[addr - 0x8000] = val;
    else if (0xA000 <= addr && addr <= 0xBFFF) {
        // cartridge RAM
    }
    else if (0xC000 <= addr && addr <= 0xDFFF) mmu->gb->wram[addr - 0xC000] = val;
    else if (0xE000 <= addr && addr <= 0xFDFF) mmu->gb->wram[addr - 0xE000] = val;
    else if (0xFE00 <= addr && addr <= 0xFE9F) mmu->gb->oam[addr - 0xFE00] = val;
    else if (0xFF00 <= addr && addr <= 0xFF7F) mmu->gb->io_registers[addr - 0xFF00] = val;
    else if (0xFF80 <= addr && addr <= 0xFFFE) mmu->gb->hram[addr - 0xFF90] = val;
    else if (addr == 0xFFFF) mmu->gb->ie = val;
}
