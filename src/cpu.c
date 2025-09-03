#include "cpu.h"

#include "gameboy.h"
#include "mmu.h"

uint8_t read_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = 0x00;

    switch (reg) {
        case REG8_B: val = cpu->b; break;
        case REG8_C: val = cpu->c; break;
        case REG8_D: val = cpu->d; break;
        case REG8_E: val = cpu->e; break;
        case REG8_H: val = cpu->h; break;
        case REG8_L: val = cpu->l; break;
        case REG8_HLMEM: val = read_byte(&cpu->gameboy->mmu, read_r16(cpu, REG16_HL)); break;
        case REG8_A: val = cpu->a; break;
    }

    return val;
}

uint16_t read_r16(CPU *cpu, Reg16 reg) {
    uint8_t hi = 0x00, lo= 0x00;

    switch (reg) {
        case REG16_BC:
            hi = cpu->b;
            lo = cpu->c;
            break; 
        case REG16_DE:
            hi = cpu->d;
            lo = cpu->e;
            break; 
        case REG16_HL:
            hi = cpu->h;
            lo = cpu->l;
            break; 
        case REG16_SP:
            return cpu->sp; 
        case REG16_AF:
            hi = cpu->a;
            lo = cpu->f;
            break;
    }

    return (0x0000 | ((uint16_t)hi << 8) | (uint16_t)lo);
}

void write_r8(CPU *cpu, Reg8 reg, uint8_t val) {
    switch (reg) {
        case REG8_B: cpu->b = val; break;
        case REG8_C: cpu->c = val; break;
        case REG8_D: cpu->d = val; break;
        case REG8_E: cpu->e = val; break;
        case REG8_H: cpu->h = val; break;
        case REG8_L: cpu->l = val; break;
        case REG8_HLMEM: write_byte(&cpu->gameboy->mmu, read_r16(cpu, REG16_HL), val); break;
        case REG8_A: cpu->a = val; break;
    }
}

void write_r16(CPU *cpu, Reg16 reg, uint16_t val) {
    uint8_t hi = (uint8_t)(val >> 8), lo = (uint8_t)val;

    switch (reg) {
        case REG16_BC:
            cpu->b = hi;
            cpu->c = lo;
            break;
        case REG16_DE:
            cpu->d = hi;
            cpu->e = lo;
        case REG16_HL:
            cpu->h = hi;
            cpu->l = lo;
            break;
        case REG16_SP:
            cpu->sp = val;
            break;
        case REG16_AF:
            cpu->a = hi;
            cpu->f = lo;
            break;
    }
}

void clear_flags(CPU *cpu) {
    cpu->f = 0x00;
}

uint8_t get_flag(CPU *cpu, Flag flag) {
    return (cpu->f >> flag) & 0x01;
}

void set_flag(CPU *cpu, Flag flag, uint8_t val) {
    if (val == 0) cpu->f &= ~(0x01 << flag);
    else cpu->f |= 0x01 << flag;
}

uint8_t evaluate_condition(CPU *cpu, Condition cond) {
    // thats how the condition can be checked directly from the opcode
    return get_flag(cpu, 7 - (3 * (cond >> 1))) == (cond & 0x01);
}



void ld_r8_r8(CPU *cpu, Reg8 dest, Reg8 src) {
    write_r8(cpu, dest, read_r8(cpu, src));
}

void ld_r8_n8(CPU *cpu, Reg8 dest, uint8_t val) {
    write_r8(cpu, dest, val);
}

void ld_r16_n16(CPU *cpu, Reg16 dest, uint16_t val) {
    write_r16(cpu, dest, val);
}

void ld_r16mem_a(CPU *cpu, Reg16 dest) {
    write_byte(&cpu->gameboy->mmu, read_r16(cpu, dest), cpu->a);
}

void ld_a16_a(CPU *cpu, uint16_t dest) {
    write_byte(&cpu->gameboy->mmu, dest, cpu->a);
}

void ld_a_r16mem(CPU *cpu, Reg16 src) {
    cpu->a = read_byte(&cpu->gameboy->mmu, read_r16(cpu, src));
}

void ld_a_a16(CPU *cpu, uint16_t src) {
    cpu->a = read_byte(&cpu->gameboy->mmu, src);
}

void ldh_a8_a(CPU *cpu, uint8_t dest) {
    write_byte(&cpu->gameboy->mmu, 0xFF00 + (uint16_t)dest, cpu->a);
}

void ldh_cmem_a(CPU *cpu) {
    write_byte(&cpu->gameboy->mmu, 0xFF00 + (uint16_t)cpu->c, cpu->a);
}

void ldh_a_a8(CPU *cpu, uint8_t src) {
    cpu->a = read_byte(&cpu->gameboy->mmu, 0xFF00 + (uint16_t)src);
}

void ldh_a_cmem(CPU *cpu) {
    cpu->a = read_byte(&cpu->gameboy->mmu, 0xFF00 + (uint16_t)cpu->c);
}

void ldi_hlmem_a(CPU *cpu) {
    uint16_t hl = read_r16(cpu, REG16_HL);

    write_byte(&cpu->gameboy->mmu, hl, cpu->a);

    write_r16(cpu, REG16_HL, hl + 1);
}

void ldd_hlmem_a(CPU *cpu) {
    uint16_t hl = read_r16(cpu, REG16_HL);

    write_byte(&cpu->gameboy->mmu, hl, cpu->a);

    write_r16(cpu, REG16_HL, hl - 1);
}

void ldi_a_hlmem(CPU *cpu) {
    uint16_t hl = read_r16(cpu, REG16_HL);

    cpu->a = read_byte(&cpu->gameboy->mmu, hl);

    write_r16(cpu, REG16_HL, hl + 1);
}

void ldd_a_hlmem(CPU *cpu) {
    uint16_t hl = read_r16(cpu, REG16_HL);

    cpu->a = read_byte(&cpu->gameboy->mmu, hl);

    write_r16(cpu, REG16_HL, hl - 1);
}

void ld_sp_n16(CPU *cpu, uint16_t val) {
    cpu->sp = val;
}

void ld_a16_sp(CPU *cpu, uint16_t dest) {
    write_word(&cpu->gameboy->mmu, dest, cpu->sp);
}

void ld_hl_sp_e8(CPU *cpu, uint8_t val) {
    write_r16(cpu, REG16_HL, cpu->sp += (int8_t)val);
}

void ld_sp_hl(CPU *cpu) {
    cpu->sp = read_r16(cpu, REG16_HL);
}
