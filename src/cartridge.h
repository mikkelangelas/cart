#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>

typedef enum CartType {
    CART_NO_MBC,
    CART_MBC1,
    CART_MBC2,
    CART_MBC3,
    CART_UNKNOWN
} CartType;

typedef struct Cartridge {
    CartType type;

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
