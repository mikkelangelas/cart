#include "gameboy.h"
#include "cartridge.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

#include <stdlib.h>
#include <string.h>

Gameboy *create_gameboy(const char *rom_file) {
    Gameboy *new_gb = (Gameboy*)malloc(sizeof(Gameboy));

    cpu_init(&new_gb->cpu, new_gb);
    mmu_init(&new_gb->mmu, new_gb);
    ppu_init(&new_gb->ppu, new_gb);

    memset(new_gb->framebuffer, 0x00, GB_SCREEN_W * GB_SCREEN_H);

    memset(new_gb->vram, 0x00, VRAM_SIZE);
    memset(new_gb->wram, 0x00, WRAM_SIZE);
    memset(new_gb->oam, 0x00, OAM_SIZE);
    memset(new_gb->io_registers, 0x00, IO_REGISTERS_SIZE);
    memset(new_gb->hram, 0x00, HRAM_SIZE);
    new_gb->ie = 0x00;

    new_gb->cartridge = create_cartridge(rom_file);

    return new_gb;
}

void destroy_gameboy(Gameboy *gb) {
    if (gb == NULL) return;

    destroy_cartridge(gb->cartridge);
    free(gb);
}

void gameboy_step(Gameboy *gb) {
    uint8_t cpu_cycles = cpu_step(&gb->cpu);
    ppu_step(&gb->ppu, cpu_cycles);
}
