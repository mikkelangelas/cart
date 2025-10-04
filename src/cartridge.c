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
    uint8_t val = 0xFF;

    switch (addr & 0xF000) {
        case 0x0000: // fixed ROM bank
        case 0x1000:
        case 0x2000:
        case 0x3000:

            break;

        case 0x4000: // switchable ROM bank
        case 0x5000:
        case 0x6000:
        case 0x7000:
            break;

        case 0xA000: // switchable RAM bank
        case 0xB000:
            break;
    }

    return val;
}

static uint8_t cartridge_read_mbc2(Cartridge *cart, uint16_t addr) {
    uint8_t val = 0xFF;

    switch (addr & 0xF000) {
        case 0x0000: // fixed ROM bank
        case 0x1000:
        case 0x2000:
        case 0x3000:
            val = cart->rom[addr];
            break;

        case 0x4000: // switchable ROM bank
        case 0x5000:
        case 0x6000:
        case 0x7000:
            val = cart->rom[(addr - CART_ROM_BASE_ADDR) | (max(cart->primary_bank, 0x01) << 14)];
            break;

        case 0xA000: // built-in RAM bank and its 'echoes'
        case 0xB000:
            val = cart->ram[(addr - CART_RAM_BASE_ADDR) % 0x0200];
            break;
    }

    return val;
}

static uint8_t cartridge_read_mbc3(Cartridge *cart, uint16_t addr) {
    uint8_t val = 0xFF;

    switch (addr & 0xF000) {
        case 0x0000: // fixed ROM bank
        case 0x1000:
        case 0x2000:
        case 0x3000:
            break;

        case 0x4000: // switchable ROM bank
        case 0x5000:
        case 0x6000:
        case 0x7000:
            break;

        case 0xA000: // switchable RAM bank or RTC register
        case 0xB000:
            break;
    }
    
    return val;
}

static void cartridge_write_no_mbc(Cartridge *cart, uint16_t addr, uint8_t val) {
    if (0xA000 <= addr && addr <= 0xBFFF && (addr - 0xA000) <= cart->ram_size)
        cart->ram[addr - 0xA000] = val;
}

static void cartridge_write_mbc1(Cartridge *cart, uint16_t addr, uint8_t val) {
    switch (addr & 0xF000) {
        case 0x0000: // RAM enable register
        case 0x1000:
            cart->ram_enable = val & MBC1_RAM_ENABLE_MASK;
            break;

        case 0x2000: // primary bank number register (ROM)
        case 0x3000:
            cart->primary_bank = val & MBC1_PRIMARY_BANK_MASK;
            break;

        case 0x4000: // secondary bank number register (RAM or upper bits of ROM)
        case 0x5000:
            cart->secondary_bank = val & MBC1_SECONDARY_BANK_MASK;
            break;

        case 0x6000: // banking mode select register
        case 0x7000:
            cart->banking_mode = val;
            break;

        case 0xA000: // switchable RAM bank
        case 0xB000:
            if (!cart->ram_enable) return;

            // TODO -> RAM writes

            break;
    }
}

static void cartridge_write_mbc2(Cartridge *cart, uint16_t addr, uint8_t val) {
    switch (addr & 0xF000) {
        case 0x0000: // RAM enable and primary bank number register
        case 0x1000:
        case 0x2000:
        case 0x3000:
            if (addr & 0x0100) cart->primary_bank = val & MBC2_PRIMARY_BANK_MASK;
            else cart->ram_enable = val & MBC2_RAM_ENABLE_MASK;
            break;

        case 0xA000: // built-in RAM bank and its 'echoes'
        case 0xB000:
            cart->ram[(addr - CART_RAM_BASE_ADDR) % 0x0200] = val;
            break;
    }
}

static void cartridge_write_mbc3(Cartridge *cart, uint16_t addr, uint8_t val) {
    switch (addr & 0xF000) {
        case 0x0000: // RAM and timer enable register
        case 0x1000:
            break;

        case 0x2000: // primary bank number register (ROM)
        case 0x3000:
            break;

        case 0x4000: // secondary bank number register (RAM or RTC)
        case 0x5000:
            break;

        case 0x6000: // latch clock data register
        case 0x7000:
            break;

        case 0xA000: // switchable RAM bank or RTC
        case 0xB000:
            break;
    }
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
    new_cart->banking_mode = 0;

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
