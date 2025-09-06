#ifndef GAMEBOY_H
#define GAMEBOY_H

#include <stdint.h>

#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

#include "cartridge.h"

#define GB_SCREEN_W 160
#define GB_SCREEN_H 144

//      memory addresses:    end      start
#define VRAM_SIZE           (0x9FFF - 0x8000 + 1)
#define WRAM_SIZE           (0xDFFF - 0xC000 + 1)
#define OAM_SIZE            (0xFE9F - 0xFE00 + 1)
#define IO_REGISTERS_SIZE   (0xFF7F - 0xFF00 + 1)
#define HRAM_SIZE           (0xFFFE - 0xFF80 + 1)

#define IF_ADDR 0xFF0F
#define IE_ADDR 0xFFFF

typedef enum Interrupt {
    INTERRUPT_VBLANK = 0,
    INTERRUPT_LCD = 1,
    INTERRUPT_TIMER = 2,
    INTERRUPT_SERIAL = 3,
    INTERRUPT_JOYPAD = 4
} Interrupt;

typedef struct Gameboy {
    // main components
    CPU cpu; 
    MMU mmu;
    PPU ppu;

    uint8_t framebuffer[GB_SCREEN_W * GB_SCREEN_H];

    // specific memory areas
    uint8_t vram[VRAM_SIZE];
    uint8_t wram[WRAM_SIZE];
    uint8_t oam[OAM_SIZE];
    uint8_t io_registers[IO_REGISTERS_SIZE];
    uint8_t hram[HRAM_SIZE];
    uint8_t ie;

    Cartridge *cartridge;
} Gameboy;

Gameboy *create_gameboy(const char *rom_file);

void destroy_gameboy(Gameboy *gb);

void gameboy_step(Gameboy *gb);

void gameboy_interrupt(Gameboy *gb, Interrupt intr);

#endif
