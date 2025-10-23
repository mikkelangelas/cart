#ifndef GAMEBOY_H
#define GAMEBOY_H

#include <stdint.h>

#include "cpu.h"
#include "mmu.h"
#include "ppu.h"
#include "joypad.h"
#include "timer.h"

#include "cartridge.h"

#define GB_SCREEN_W 160
#define GB_SCREEN_H 144

//      memory addresses:    end      start
#define VRAM_SIZE           (0x9FFF - 0x8000 + 1)
#define WRAM_SIZE           (0xDFFF - 0xC000 + 1)
#define OAM_SIZE            (0xFE9F - 0xFE00 + 1)
#define IO_SIZE             (0xFF7F - 0xFF00 + 1)
#define HRAM_SIZE           (0xFFFE - 0xFF80 + 1)

#define VRAM_BASE_ADDR      0x8000
#define WRAM_BASE_ADDR      0xC000
#define WRAM_ECHO_BASE_ADDR 0xE000
#define OAM_BASE_ADDR       0xFE00
#define IO_BASE_ADDR        0xFF00
#define HRAM_BASE_ADDR      0xFF80

#define IF_ADDR 0xFF0F
#define IE_ADDR 0xFFFF

#define IF_ADDR_RELATIVE 0x000F

typedef enum Interrupt {
    INTERRUPT_VBLANK = 0,
    INTERRUPT_STAT   = 1,
    INTERRUPT_TIMER  = 2,
    INTERRUPT_SERIAL = 3,
    INTERRUPT_JOYPAD = 4
} Interrupt;

typedef struct GB {
    // main components
    CPU cpu; 
    MMU mmu;
    PPU ppu;
    Joypad joypad;
    Timer timer;

    uint8_t framebuffer[GB_SCREEN_W * GB_SCREEN_H];
    uint8_t frame_ready;

    // specific memory areas
    uint8_t vram[VRAM_SIZE];
    uint8_t wram[WRAM_SIZE];
    uint8_t oam[OAM_SIZE];
    uint8_t io[IO_SIZE];
    uint8_t hram[HRAM_SIZE];
    uint8_t ie;

    Cartridge *cartridge;
} GB;

GB *create_gb(const char *rom_file);

void destroy_gb(GB *gb);

void gb_step(GB *gb);

void gb_interrupt(GB *gb, Interrupt intr);

#endif
