#include "cartridge.h"

#include <stdlib.h>
#include <stdio.h>

static CartType get_cart_type(uint8_t code) {
    CartType type = CART_UNKNOWN;

    switch (code) {
        case 0x00:
        case 0x08:
        case 0x09:
            type = CART_NO_MBC;
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            type = CART_MBC1;
            break;
        case 0x05:
        case 0x06:
            type = CART_MBC2;
            break;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            type = CART_MBC3;
            break;
    }

    return type;
}

static uint32_t get_cart_ram_size(uint8_t code) {
    uint32_t size = 0;

    switch (code) {
        case 0x00: size = 0; break;
        case 0x02: size = 8192; break;
        case 0x03: size = 32768; break;
        case 0x04: size = 131072; break;
        case 0x05: size = 65536; break;
    }

    return size;
}

Cartridge *create_cartridge(const char *rom_file) {
    Cartridge *new_cart = (Cartridge*)malloc(sizeof(Cartridge));


    return new_cart;
}

void destroy_cartridge(Cartridge *cart) {
    if (cart == NULL)  return;

    free(cart->ram);
    free(cart->rom);
    free(cart);
}

uint8_t cartridge_read(Cartridge *cart, uint16_t addr) {
    uint8_t val = 0xFF;

    switch (cart->type) {
        case CART_NO_MBC: break;
        case CART_MBC1: break;
        case CART_MBC2: break;
        case CART_MBC3: break;
        case CART_UNKNOWN: break;
    }

    return val;
}

void cartridge_write(Cartridge *cart, uint16_t addr, uint8_t val) {
    switch (cart->type) {
        case CART_NO_MBC: break;
        case CART_MBC1: break;
        case CART_MBC2: break;
        case CART_MBC3: break;
        case CART_UNKNOWN: return;
    }
}
