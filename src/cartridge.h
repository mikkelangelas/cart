#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>

#define KIB_8   0x00002000
#define KIB_32  0x00008000
#define KIB_64  0x00010000
#define KIB_128 0x00020000
#define KIB_256 0x00040000
#define KIB_512 0x00080000
#define MIB_1   0x00100000
#define MIB_2   0x00200000
#define MIB_4   0x00400000
#define MIB_8   0x00800000

#define CART_TYPE_ADDR 0x147
#define CART_ROM_SIZE_ADDR 0x148
#define CART_RAM_SIZE_ADDR 0x149

typedef enum CartridgeType {
    CART_TYPE_NO_MBC,
    CART_TYPE_MBC1,
    CART_TYPE_MBC2,
    CART_TYPE_MBC3,
    CART_TYPE_UNKNOWN
} CartridgeType;

typedef struct Cartridge {
    CartridgeType type;

    uint8_t *rom;
    uint8_t *ram;

    uint32_t rom_size;
    uint32_t ram_size;

    uint8_t ram_enable;
    uint8_t primary_bank;
    uint8_t secondary_bank;
    uint8_t mbc1_advanced;
} Cartridge;

Cartridge *create_cartridge(const char *rom_file);

void destroy_cartridge(Cartridge *cart);

uint8_t cartridge_read(Cartridge *cart, uint16_t addr);
void cartridge_write(Cartridge *cart, uint16_t addr, uint8_t val);

#endif
