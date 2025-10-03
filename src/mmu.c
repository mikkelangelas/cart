#include "mmu.h"

#include "gb.h"
#include "bootrom.h"

void mmu_init(MMU *mmu, struct GB *gb) {
    *mmu = (MMU){
        .bootrom_mapped = 1,
        .gb = gb
    };
}

uint8_t mmu_read(MMU *mmu, uint16_t addr) {
    uint8_t val = 0xFF;

    switch (addr & 0xF000) {
        case 0x0000:
            if (mmu->bootrom_mapped == 1 && addr < 0x0100) {
               val = DMG_BOOTROM[addr];
                break;
            }
        case 0x1000: // fallthrough if not reading from bootrom
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            val = cartridge_read(mmu->gb->cartridge, addr); break;

        case 0x8000:
        case 0x9000:
            val = mmu->gb->vram[addr - VRAM_BASE_ADDR]; break;

        case 0xA000:
        case 0xB000:
            val = cartridge_read(mmu->gb->cartridge, addr); break;

        case 0xC000:
        case 0xD000:
            val = mmu->gb->wram[addr - WRAM_BASE_ADDR]; break;

        case 0xE000:
        case 0xF000:
            if (addr <= 0xFDFF)
                val = mmu->gb->wram[addr - WRAM_ECHO_BASE_ADDR];
            else if (addr <= 0xFE9F)
                val = mmu->gb->oam[addr - OAM_BASE_ADDR];
            else if (addr <= 0xFEFF)
                break;
            else if (addr <= 0xFF7F)
                val = mmu->gb->io[addr - IO_BASE_ADDR];
            else if (addr <= 0xFFFE)
                val = mmu->gb->hram[addr - HRAM_BASE_ADDR];
            else
                val = mmu->gb->ie;
    }

    return val;
}

void mmu_write(MMU *mmu, uint16_t addr, uint8_t val) {
    switch (addr & 0xF000) {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            cartridge_write(mmu->gb->cartridge, addr, val); break;

        case 0x8000:
        case 0x9000:
            mmu->gb->vram[addr - VRAM_BASE_ADDR] = val; break;

        case 0xA000:
        case 0xB000:
            cartridge_write(mmu->gb->cartridge, addr, val); break;

        case 0xC000:
        case 0xD000:
            mmu->gb->wram[addr - WRAM_BASE_ADDR] = val; break;

        case 0xE000:
        case 0xF000: 
            if (addr <= 0xFDFF)
                mmu->gb->wram[addr - WRAM_ECHO_BASE_ADDR] = val;
            else if (addr <= 0xFE9F)
                mmu->gb->oam[addr - OAM_BASE_ADDR] = val;
            else if (addr <= 0xFEFF)
                return;
            else if (addr <= 0xFF7F) {
                mmu->gb->io[addr - IO_BASE_ADDR] = val;

                if (addr == JOYP_ADDR) joypad_update(&mmu->gb->joypad);
                else if (addr == DMA_ADDR) mmu_dma_transfer(mmu, val);
                else if (addr == BANK_ADDR) mmu->bootrom_mapped = 0;
            }
            else if (addr <= 0xFFFE)
                mmu->gb->hram[addr - HRAM_BASE_ADDR] = val;
            else
                mmu->gb->ie = val;
    }
}

void mmu_dma_transfer(MMU *mmu, uint8_t start) {
    if (start > 0xDF) return;

    uint16_t src = start << 8;

    for (uint8_t b = 0; b < OAM_SIZE; b++)
        mmu->gb->oam[b] = mmu_read(mmu, src + b);
}
