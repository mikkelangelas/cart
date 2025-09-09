#include "gb.h"
#include "cartridge.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

#include <stdlib.h>
#include <string.h>

GB *create_gb(const char *rom_file) {
    GB *new_gb = (GB*)malloc(sizeof(GB));

    new_gb->cartridge = create_cartridge(rom_file);

    if (new_gb->cartridge == NULL) {
        destroy_gb(new_gb);
        return NULL;
    }

    cpu_init(&new_gb->cpu, new_gb);
    mmu_init(&new_gb->mmu, new_gb);
    ppu_init(&new_gb->ppu, new_gb);

    memset(new_gb->framebuffer, 0x03, GB_SCREEN_W * GB_SCREEN_H);
    new_gb->frame_ready = 0;

    memset(new_gb->vram, 0x00, VRAM_SIZE);
    memset(new_gb->wram, 0x00, WRAM_SIZE);
    memset(new_gb->oam, 0x00, OAM_SIZE);
    memset(new_gb->io_registers, 0x00, IO_REGISTERS_SIZE);
    memset(new_gb->hram, 0x00, HRAM_SIZE);
    new_gb->ie = 0x00;

    return new_gb;
}

void destroy_gb(GB *gb) {
    if (gb == NULL) return;

    destroy_cartridge(gb->cartridge);
    free(gb);
}

void gb_step(GB *gb) {
    uint8_t cpu_cycles = cpu_step(&gb->cpu);
    ppu_step(&gb->ppu, cpu_cycles);
}

void gb_interrupt(GB *gb, Interrupt intr) {
    mmu_write(&gb->mmu, IF_ADDR, mmu_read(&gb->mmu, IF_ADDR) | (0x01 << intr));
}
