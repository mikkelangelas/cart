#include "mmu.h"

#include "gameboy.h"

uint8_t read_byte(MMU *mmu, uint16_t addr) {
    uint8_t val = 0xFF;

    if (0x0000 <= addr && addr <= 0x7FFF) {
        // cartridge ROM 
    }
    else if (0x8000 <= addr && addr <= 0x9FFF) val = mmu->gameboy->vram[addr - 0x8000];
    else if (0xA000 <= addr && addr <= 0xBFFF) {
        // cartridge RAM
    }
    else if (0xC000 <= addr && addr <= 0xDFFF) val = mmu->gameboy->wram[addr - 0xC000];
    else if (0xE000 <= addr && addr <= 0xFDFF) val = mmu->gameboy->wram[addr - 0xE000];
    else if (0xFE00 <= addr && addr <= 0xFE9F) val = mmu->gameboy->oam[addr - 0xFE00];
    else if (0xFEA0 <= addr && addr <= 0xFEFF) {
        // prohibited, used to emulate OAM corruption bug
        return 0xFF;
    }
    else if (0xFF00 <= addr && addr <= 0xFF7F) val = mmu->gameboy->io_registers[addr - 0xFF00];
    else if (0xFF80 <= addr && addr <= 0xFFFE) val = mmu->gameboy->hram[addr - 0xFF90];
    else if (addr == 0xFFFF) val = mmu->gameboy->ie;

    return val;
}

uint16_t read_word(MMU *mmu, uint16_t addr) {
    return ((uint16_t)read_byte(mmu, addr + 1) << 8) | (uint16_t)read_byte(mmu, addr);
}

void write_byte(MMU *mmu, uint16_t addr, uint8_t val) {
    if (0x0000 <= addr && addr <= 0x7FFF) {
        // cartridge ROM 
    }
    else if (0x8000 <= addr && addr <= 0x9FFF) mmu->gameboy->vram[addr - 0x8000] = val;
    else if (0xA000 <= addr && addr <= 0xBFFF) {
        // cartridge RAM
    }
    else if (0xC000 <= addr && addr <= 0xDFFF) mmu->gameboy->wram[addr - 0xC000] = val;
    else if (0xE000 <= addr && addr <= 0xFDFF) mmu->gameboy->wram[addr - 0xE000] = val;
    else if (0xFE00 <= addr && addr <= 0xFE9F) mmu->gameboy->oam[addr - 0xFE00] = val;
    else if (0xFF00 <= addr && addr <= 0xFF7F) mmu->gameboy->io_registers[addr - 0xFF00] = val;
    else if (0xFF80 <= addr && addr <= 0xFFFE) mmu->gameboy->hram[addr - 0xFF90] = val;
    else if (addr == 0xFFFF) mmu->gameboy->ie = val;
}

void write_word(MMU *mmu, uint16_t addr, uint16_t val) {
    write_byte(mmu, addr, (uint8_t)val);
    write_byte(mmu, addr + 1, (uint8_t)(val >> 8));
}
