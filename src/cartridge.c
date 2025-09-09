#include "cartridge.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

static CartridgeType get_cart_type(uint8_t code) {
    CartridgeType type = CART_TYPE_UNKNOWN;

    switch (code) {
        case 0x00:
        case 0x08:
        case 0x09:
            type = CART_TYPE_NO_MBC;
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            type = CART_TYPE_MBC1;
            break;
        case 0x05:
        case 0x06:
            type = CART_TYPE_MBC2;
            break;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            type = CART_TYPE_MBC3;
            break;
    }

    return type;
}

static uint32_t get_cart_ram_size(uint8_t code) {
    uint32_t size = 0x00;

    switch (code) {
        case 0x00:
            size = 0x00; break;
        case 0x02:
            size = KIB_8; break;
        case 0x03:
            size = KIB_32; break;
        case 0x04:
            size = KIB_128; break;
        case 0x05:
            size = KIB_64; break;
    }

    return size;
}

static uint32_t get_cart_rom_size(uint8_t code) {
    uint32_t size = 0x00;

    switch(code) {
        case 0x00:
            size = KIB_32; break;
        case 0x01:
            size = KIB_64; break;
        case 0x02:
            size = KIB_128; break;
        case 0x03:
            size = KIB_256; break;
        case 0x04:
            size = KIB_512; break;
        case 0x05:
            size = MIB_1; break;
        case 0x06:
            size = MIB_2; break;
        case 0x07:
            size = MIB_4; break;
        case 0x08:
            size = MIB_8; break;
    }

    return size;
}

static uint8_t cartridge_read_no_mbc(Cartridge *cart, uint16_t addr) {
    uint16_t val = 0xFF;

    if (addr <= 0x7FFF && addr < cart->rom_size)
        val = cart->rom[addr];
    else if (0xA000 <= addr && addr <= 0xBFFF && (addr - 0xA000) < cart->ram_size)
        val = cart->ram[addr - 0xA000];

    return val;
}

static uint8_t cartridge_read_mbc1(Cartridge *cart, uint16_t addr) {

}

static uint8_t cartridge_read_mbc2(Cartridge *cart, uint16_t addr) {

}

static uint8_t cartridge_read_mbc3(Cartridge *cart, uint16_t addr) {

}

static void cartridge_write_no_mbc(Cartridge *cart, uint16_t addr, uint8_t val) {
    if (0xA000 <= addr && addr <= 0xBFFF && (addr - 0xA000) <= cart->ram_size)
        cart->ram[addr - 0xA000] = val;
}

static void cartridge_write_mbc1(Cartridge *cart, uint16_t addr, uint8_t val) {

}

static void cartridge_write_mbc2(Cartridge *cart, uint16_t addr, uint8_t val) {

}

static void cartridge_write_mbc3(Cartridge *cart, uint16_t addr, uint8_t val) {

}



Cartridge *create_cartridge(const char *rom_file) {
    Cartridge *new_cart = (Cartridge*)malloc(sizeof(Cartridge));

    uint8_t *rom_buf = read_file_to_array(rom_file, 1);

    if (rom_buf == NULL) {
        destroy_cartridge(new_cart);
        return NULL;
    }

    new_cart->type = get_cart_type(rom_buf[CART_TYPE_ADDR]);
    new_cart->rom = rom_buf;
    new_cart->rom_size = get_cart_rom_size(rom_buf[CART_ROM_SIZE_ADDR]);
    new_cart->ram_size = (new_cart->type != CART_TYPE_MBC2)
        ? get_cart_ram_size(rom_buf[CART_RAM_SIZE_ADDR])
        : 0x200;

    new_cart->ram = (uint8_t*)malloc(new_cart->ram_size);

    new_cart->ram_enable = 0;
    new_cart->primary_bank = 0x00;
    new_cart->secondary_bank = 0x00;
    new_cart->mbc1_advanced = 0;

    memset(new_cart->ram, 0, new_cart->ram_size);

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
        case CART_TYPE_NO_MBC:
            val = cartridge_read_no_mbc(cart, addr); break;
        case CART_TYPE_MBC1:
            val = cartridge_read_mbc1(cart, addr); break;
        case CART_TYPE_MBC2:
            val = cartridge_read_mbc2(cart, addr); break;
        case CART_TYPE_MBC3:
            val = cartridge_read_mbc3(cart, addr); break;
        case CART_TYPE_UNKNOWN: break;
    }

    return val;
}


void cartridge_write(Cartridge *cart, uint16_t addr, uint8_t val) {
    switch (cart->type) {
        case CART_TYPE_NO_MBC:
            cartridge_write_no_mbc(cart, addr, val); break;
        case CART_TYPE_MBC1:
            cartridge_write_mbc1(cart, addr, val); break;
        case CART_TYPE_MBC2:
            cartridge_write_mbc2(cart, addr, val); break;
        case CART_TYPE_MBC3:
            cartridge_write_mbc3(cart, addr, val); break;
        case CART_TYPE_UNKNOWN: return;
    }
}
