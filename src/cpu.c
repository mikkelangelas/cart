#include "cpu.h"

#include "gb.h"
#include "util.h"
#include "opcodes.h"
#include <stdio.h>

uint8_t print_debug = 0;

extern inline uint8_t get_bit(uint8_t src, uint8_t bit);
extern inline void set_bit(uint8_t *dest, uint8_t bit, uint8_t val);

// --------------------------
//      helper functions
// --------------------------

static inline uint8_t get_flag(CPU *cpu, CPUFlag flag) {
    return get_bit(cpu->f, flag);
}

static inline void set_flags_sp_e8(CPU *cpu, uint8_t op) {
    cpu->f = 0x00 
        | (((uint8_t)cpu->sp & 0x0F) + (op & 0x0F) > 0x0F) << CPU_FLAG_H
        | (cpu->sp + (uint16_t)op > 0x00FF) << CPU_FLAG_C;
}

static inline void set_flags_addition(CPU *cpu, uint8_t op1, uint8_t op2) {
    cpu->f = 0x00
        | ((uint8_t)(op1 + op2) == 0x00) << CPU_FLAG_Z
        | ((op1 & 0x0F) + (op2 & 0x0F) > 0x0F) << CPU_FLAG_H
        | ((uint16_t)op1 + (uint16_t)op2 > 0x00FF) << CPU_FLAG_C;
}

static inline void set_flags_subtraction(CPU *cpu, uint8_t op1, uint8_t op2) {
    cpu->f = 0x40 
        | ((uint8_t)(op1 - op2) == 0x00) << CPU_FLAG_Z
        | ((op2 & 0x0F) > (op1 & 0x0F)) << CPU_FLAG_H
        | (op2 > op1) << CPU_FLAG_C;
}

static inline void set_flags_and(CPU *cpu, uint8_t res) {
    cpu->f = 0x20 | ((res == 0x00) << CPU_FLAG_Z);
}

static inline void set_flags_or_xor(CPU *cpu, uint8_t res) {
    cpu->f = 0x00 | ((res == 0x00) << CPU_FLAG_Z);
}

static inline void set_flags_roll_a(CPU *cpu, uint8_t carry) {
    cpu->f = 0x00 | (carry << CPU_FLAG_C);
}

static inline void set_flags_roll_shift(CPU *cpu, uint8_t carry, uint8_t res) {
    cpu->f = (carry << CPU_FLAG_C) | ((res == 0x00) << CPU_FLAG_Z);
}

static inline uint8_t evaluate_condition(CPU *cpu, CPUCondition cond) {
    // thats how the condition can be checked directly from the opcode
    return get_flag(cpu, CPU_FLAG_Z - (3 * (cond >> 1))) == (cond & 0x01);

}

static inline uint8_t pc_read_byte(CPU *cpu) {
    uint8_t byte = mmu_read(&cpu->gb->mmu, cpu->pc++);

    if (0)
        printf(
                "%x fetch byte: %x a: %x b: %x c: %x d: %x e: %x h: %x l: %x sp: %x flags: %x\n",
                cpu->pc-1,
                byte,
                cpu->a,
                cpu->b, 
                cpu->c,
                cpu->d,
                cpu->e,
                cpu->h,
                cpu->l,
                cpu->sp,
                cpu->f
        );


    return byte; 
}


static inline uint16_t pc_read_word(CPU *cpu) {
    return (uint16_t)pc_read_byte(cpu) | ((uint16_t)pc_read_byte(cpu) << 8);
}



void cpu_init(CPU *cpu, struct GB *gb) {
    *cpu = (CPU){
        .a = 0x00,
        .f = 0x00,
        .b = 0x00,
        .c = 0x00,
        .d = 0x00,
        .e = 0x00,
        .h = 0x00,
        .l = 0x00,
        .pc = 0x0000,
        .sp = 0x0000,
        .ime = 0,
        .halted = 0,
        .gb = gb
    };
}

uint8_t cpu_step(CPU *cpu) {
    uint8_t cycles = cpu_handle_interrupts(cpu);

    if (cycles > 0) return cycles;
    if (cpu->halted == 1) return 1;

    uint8_t opcode = pc_read_byte(cpu);

    cycles = (opcode != 0xCB)
        ? cpu_execute(cpu, opcode)
        : cpu_execute_prefixed(cpu, pc_read_byte(cpu));

    if (cpu->ime_set_pending > 0) {
        if (cpu->ime_set_pending == 1) cpu->ime = 1;
        cpu->ime_set_pending--;
    }

    return cycles;
}

uint8_t cpu_execute(CPU *cpu, uint8_t opcode) {
    uint8_t extra_cycles = 0;

    if (opcode <= 0x3F) {                                                        // block 0
        switch (opcode & 0x0F) {
            case 0x00:
            case 0x08:
                switch (opcode) {
                    case 0x00:
                        nop(cpu); break;
                    case 0x10:
                        stop(cpu, pc_read_byte(cpu)); break;
                    case 0x08:
                        ld_a16_sp(cpu, pc_read_word(cpu)); break;
                    case 0x18: 
                        jr_e8(cpu, pc_read_byte(cpu)); break;
                    default:
                        extra_cycles = jr_cond_e8(cpu, (opcode >> 3) & 0x03, pc_read_byte(cpu)); break;
                }
                break;

            case 0x01:
                ld_r16_n16(cpu, (opcode >> 4) & 0x03, pc_read_word(cpu)); break;

            case 0x02:
                switch (opcode) {
                    case 0x02:
                    case 0x12:
                        ld_r16mem_a(cpu, (opcode >> 4) & 0x03); break;
                    case 0x22:
                        ldi_hlmem_a(cpu); break;
                    case 0x32:
                        ldd_hlmem_a(cpu); break;
                }
                break;

            case 0x03:
                inc_r16(cpu, (opcode >> 4) & 0x03); break;

            case 0x04:
            case 0x0C:
                inc_r8(cpu, (opcode >> 3) & 0x07); break;

            case 0x05:
            case 0x0D:
                dec_r8(cpu, (opcode >> 3) & 0x07); break;

            case 0x06:
            case 0x0E:
                ld_r8_n8(cpu, (opcode >> 3) & 0x07, pc_read_byte(cpu)); break;

            case 0x07:
                switch (opcode) {
                    case 0x07:
                        rlca(cpu); break;
                    case 0x17:
                        rla(cpu); break;
                    case 0x27:
                        daa(cpu); break;
                    case 0x37:
                        scf(cpu); break;
                }
                break;

            case 0x09:
                add_hl_r16(cpu, (opcode >> 4) & 0x03); break;

            case 0x0A:
                switch (opcode) {
                    case 0x0A:
                    case 0x1A:
                        ld_a_r16mem(cpu, (opcode >> 4) & 0x03); break;
                    case 0x2A:
                        ldi_a_hlmem(cpu); break;
                    case 0x3A:
                        ldd_a_hlmem(cpu); break;
                }
                break;

            case 0x0B:
                dec_r16(cpu, (opcode >> 4) & 0x03); break;

            case 0x0F:
                switch (opcode) {
                    case 0x0F:
                        rrca(cpu); break;
                    case 0x1F:
                        rra(cpu); break;
                    case 0x2F:
                        cpl(cpu); break;
                    case 0x3F:
                        ccf(cpu); break;
                }
        }

    }
    else if (opcode <= 0x75) ld_r8_r8(cpu, (opcode >> 3) & 0x07, opcode & 0x07); // block 1
    else if (opcode == 0x76) halt(cpu);
    else if (opcode <= 0x7F) ld_r8_r8(cpu, (opcode >> 3) & 0x07, opcode & 0x07);
    else if (opcode <= 0x87) add_a_r8(cpu, opcode & 0x07);                       // block 2
    else if (opcode <= 0x8F) adc_a_r8(cpu, opcode & 0x07);
    else if (opcode <= 0x97) sub_a_r8(cpu, opcode & 0x07); 
    else if (opcode <= 0x9F) sbc_a_r8(cpu, opcode & 0x07);
    else if (opcode <= 0xA7) and_a_r8(cpu, opcode & 0x07);
    else if (opcode <= 0xAF) xor_a_r8(cpu, opcode & 0x07);
    else if (opcode <= 0xB7) or_a_r8(cpu, opcode & 0x07);
    else if (opcode <= 0xBF) cp_a_r8(cpu, opcode & 0x07);
    else {                                                                       // block 3
        switch (opcode & 0x0F) {
            case 0x00:
            case 0x08:
                switch (opcode) {
                    case 0xE0:
                        ldh_a8_a(cpu, pc_read_byte(cpu)); break;
                    case 0xF0:
                        ldh_a_a8(cpu, pc_read_byte(cpu)); break;
                    case 0xE8:
                        add_sp_e8(cpu, pc_read_byte(cpu)); break;
                    case 0xF8:
                        ld_hl_sp_e8(cpu, pc_read_byte(cpu)); break;
                    default:
                        extra_cycles = ret_cond(cpu, (opcode >> 3) & 0x03) * 3; break;
                }
                break;

            case 0x01: {
                // since 2 enum variants can't have the same value,
                // a check and adjustment is needed
                Reg16 r16 = (opcode >> 4) & 0x03;
                pop_r16(cpu, r16 == 0x03 ? 0x04 : r16);
                break;
            }

            case 0x02: case 0x0A:
                switch (opcode) {
                    case 0xE2:
                        ldh_cmem_a(cpu); break;
                    case 0xF2:
                        ldh_a_cmem(cpu); break;
                    case 0xEA:
                        ld_a16_a(cpu, pc_read_word(cpu)); break;
                    case 0xFA:
                        ld_a_a16(cpu, pc_read_word(cpu)); break;
                    default:
                        extra_cycles = jp_cond_a16(cpu, (opcode >> 3) & 0x03, pc_read_word(cpu)); break;
                }
                break;

            case 0x03:
                switch (opcode) {
                    case 0xC3:
                        jp_a16(cpu, pc_read_word(cpu)); break;
                    case 0xF3:
                        di(cpu); break;
                }
                break;

            case 0x04:
            case 0x0C:
                extra_cycles = call_cond_a16(cpu, (opcode >> 3) & 0x03, pc_read_word(cpu)) * 3; break;

            case 0x05: {
                // since 2 enum variants can't have the same value,
                // a check and adjustment is needed
                Reg16 r16 = (opcode >> 4) & 0x03;
                push_r16(cpu, r16 == 0x03 ? 0x04 : r16);
                break;
            }

            case 0x06:
                switch (opcode) {
                    case 0xC6:
                        add_a_n8(cpu, pc_read_byte(cpu)); break;
                    case 0xD6:
                        sub_a_n8(cpu, pc_read_byte(cpu)); break;
                    case 0xE6:
                        and_a_n8(cpu, pc_read_byte(cpu)); break;
                    case 0xF6:
                        or_a_n8(cpu, pc_read_byte(cpu)); break;
                }
                break;

            case 0x07:
            case 0x0F:
                rst_vec(cpu, opcode & 0x38); break;

            case 0x09:
                switch (opcode) {
                    case 0xC9:
                        ret(cpu); break;
                    case 0xD9:
                        reti(cpu); break;
                    case 0xE9:
                        jp_hl(cpu); break;
                    case 0xF9:
                        ld_sp_hl(cpu); break;
                }
                break;

            case 0x0B:
                ei(cpu); break;

            case 0x0D:
                call_a16(cpu, pc_read_word(cpu)); break;

            case 0x0E:
                switch (opcode) {
                    case 0xCE:
                        adc_a_n8(cpu, pc_read_byte(cpu)); break;
                    case 0xDE:
                        sbc_a_n8(cpu, pc_read_byte(cpu)); break;
                    case 0xEE:
                        xor_a_n8(cpu, pc_read_byte(cpu)); break;
                    case 0xFE:
                        cp_a_n8(cpu, pc_read_byte(cpu)); break;
                }

                break;
        }
    }

    return OPCODES_DURATION[opcode] + extra_cycles;
}

uint8_t cpu_execute_prefixed(CPU *cpu, uint8_t opcode) {
    Reg8 reg = opcode & 0x07;

    if (opcode <= 0x07) rlc_r8(cpu, reg);
    else if (opcode <= 0x0F) rrc_r8(cpu, reg);
    else if (opcode <= 0x17) rl_r8(cpu, reg);
    else if (opcode <= 0x1F) rr_r8(cpu, reg);
    else if (opcode <= 0x27) sla_r8(cpu, reg);
    else if (opcode <= 0x2F) sra_r8(cpu, reg);
    else if (opcode <= 0x37) swap_r8(cpu, reg);
    else if (opcode <= 0x3F) srl_r8(cpu, reg);
    else {
        uint8_t bit = (opcode >> 3) & 0x07;

        if (opcode <= 0x7F) bit_r8(cpu, bit, reg);
        else if (opcode <= 0xBF) res_r8(cpu, bit, reg);
        else if (opcode <= 0xFF) set_r8(cpu, bit, reg);
    }

    return PREFIXED_OPCODES_DURATION; 
}

uint8_t cpu_handle_interrupts(CPU *cpu) {
    if (cpu->ime == 0) return 0;

    uint8_t int_flags = cpu->gb->io[IF_ADDR_RELATIVE];
    uint8_t pending = cpu->gb->ie & int_flags;

    if (pending == 0) return 0;

    cpu->halted = 0;

    for (uint8_t i = 0; i < 5; i++) {
        if (get_bit(pending, i) == 1) {
            cpu->ime = 0; // disable all interrupts

            // reset handled interrupt flag
            set_bit(&int_flags, i, 0);
            cpu->gb->io[IF_ADDR_RELATIVE] = int_flags; 

            // call interrupt handler
            push_stack(cpu, cpu->pc);
            cpu->pc = 0x0040 + (i << 3); 
        }
    }

    return 5;
}

uint8_t read_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = 0x00;

    switch (reg) {
        case REG8_B:
            val = cpu->b; break;
        case REG8_C:
            val = cpu->c; break;
        case REG8_D:
            val = cpu->d; break;
        case REG8_E:
            val = cpu->e; break;
        case REG8_H:
            val = cpu->h; break;
        case REG8_L:
            val = cpu->l; break;
        case REG8_HLMEM:
            val = mmu_read(&cpu->gb->mmu, read_r16(cpu, REG16_HL)); break;
        case REG8_A:
            val = cpu->a; break;
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

    return ((uint16_t)hi << 8) | (uint16_t)lo;
}

void write_r8(CPU *cpu, Reg8 reg, uint8_t val) {
    switch (reg) {
        case REG8_B:
            cpu->b = val; break;
        case REG8_C:
            cpu->c = val; break;
        case REG8_D:
            cpu->d = val; break;
        case REG8_E:
            cpu->e = val; break;
        case REG8_H:
            cpu->h = val; break;
        case REG8_L:
            cpu->l = val; break;
        case REG8_HLMEM:
            mmu_write(&cpu->gb->mmu, read_r16(cpu, REG16_HL), val); break;
        case REG8_A:
            cpu->a = val; break;
    }
}

void write_r16(CPU *cpu, Reg16 reg, uint16_t val) {
    uint8_t hi = (uint8_t)(val >> 8);
    uint8_t lo = (uint8_t)val;

    switch (reg) {
        case REG16_BC:
            cpu->b = hi;
            cpu->c = lo;
            break;
        case REG16_DE:
            cpu->d = hi;
            cpu->e = lo;
            break;
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

void push_stack(CPU *cpu, uint16_t val) {
    mmu_write(&cpu->gb->mmu, --cpu->sp, (uint8_t)(val >> 8));
    mmu_write(&cpu->gb->mmu, --cpu->sp, (uint8_t)val);
}

uint16_t pop_stack(CPU *cpu) {
    uint16_t val = 0x0000;

    val |= mmu_read(&cpu->gb->mmu, cpu->sp++);
    val |= mmu_read(&cpu->gb->mmu, cpu->sp++) << 8;

    return val;
}



// ---------------------------
//      load instructions
// ---------------------------

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
    mmu_write(&cpu->gb->mmu, read_r16(cpu, dest), cpu->a);
}

void ld_a16_a(CPU *cpu, uint16_t dest) {
    mmu_write(&cpu->gb->mmu, dest, cpu->a);
}

void ld_a_r16mem(CPU *cpu, Reg16 src) {
    cpu->a = mmu_read(&cpu->gb->mmu, read_r16(cpu, src));
}

void ld_a_a16(CPU *cpu, uint16_t src) {
    cpu->a = mmu_read(&cpu->gb->mmu, src);
}

void ldh_a8_a(CPU *cpu, uint8_t dest) {
    mmu_write(&cpu->gb->mmu, 0xFF00 + (uint16_t)dest, cpu->a);
}

void ldh_cmem_a(CPU *cpu) {
    mmu_write(&cpu->gb->mmu, 0xFF00 + (uint16_t)cpu->c, cpu->a);
}

void ldh_a_a8(CPU *cpu, uint8_t src) {
    cpu->a = mmu_read(&cpu->gb->mmu, 0xFF00 + (uint16_t)src);
}

void ldh_a_cmem(CPU *cpu) {
    cpu->a = mmu_read(&cpu->gb->mmu, 0xFF00 + (uint16_t)cpu->c);
}

void ldi_hlmem_a(CPU *cpu) {
    uint16_t hl = read_r16(cpu, REG16_HL);

    mmu_write(&cpu->gb->mmu, hl, cpu->a);

    write_r16(cpu, REG16_HL, hl + 1);
}

void ldd_hlmem_a(CPU *cpu) {
    uint16_t hl = read_r16(cpu, REG16_HL);

    mmu_write(&cpu->gb->mmu, hl, cpu->a);

    write_r16(cpu, REG16_HL, hl - 1);
}

void ldi_a_hlmem(CPU *cpu) {
    uint16_t hl = read_r16(cpu, REG16_HL);

    cpu->a = mmu_read(&cpu->gb->mmu, hl);

    write_r16(cpu, REG16_HL, hl + 1);
}

void ldd_a_hlmem(CPU *cpu) {
    uint16_t hl = read_r16(cpu, REG16_HL);

    cpu->a = mmu_read(&cpu->gb->mmu, hl);

    write_r16(cpu, REG16_HL, hl - 1);
}

void ld_sp_n16(CPU *cpu, uint16_t val) {
    cpu->sp = val;
}

void ld_a16_sp(CPU *cpu, uint16_t dest) {
    mmu_write(&cpu->gb->mmu, dest, (uint8_t)cpu->sp);
    mmu_write(&cpu->gb->mmu, dest + 1, (uint8_t)(cpu->sp >> 8));
}

void ld_hl_sp_e8(CPU *cpu, uint8_t val) {
    write_r16(cpu, REG16_HL, cpu->sp += (int8_t)val);
    set_flags_sp_e8(cpu, val);
}

void ld_sp_hl(CPU *cpu) {
    cpu->sp = read_r16(cpu, REG16_HL);
}



// ---------------------------------
//      arithmetic instructions
// ---------------------------------

void add_sp_e8(CPU *cpu, uint8_t val) {
    cpu->sp += (int8_t)val;
    set_flags_sp_e8(cpu, val);
}

void add_hl_r16(CPU *cpu, Reg16 reg) {
    uint16_t op1 = read_r16(cpu, REG16_HL);
    uint16_t op2 = read_r16(cpu, reg);

    write_r16(cpu, REG16_HL, op1 + op2);

    cpu->f = (cpu->f & 0x80) |
        ((op1 & 0x00FF) + (op2 & 0x00FF) > 0x00FF) << CPU_FLAG_H |
        ((uint32_t)op1 + (uint32_t)op2 > 0xFFFF) << CPU_FLAG_C;
}

void adc_a_r8(CPU *cpu, Reg8 reg) {
    uint8_t op = read_r8(cpu, reg) + get_flag(cpu, CPU_FLAG_C);

    set_flags_addition(cpu, cpu->a, op);
    cpu->a += op;
}

void adc_a_n8(CPU *cpu, uint8_t val) {
    uint8_t op = val + get_flag(cpu, CPU_FLAG_C);

    set_flags_addition(cpu, cpu->a, op);
    cpu->a += op;
}

void add_a_r8(CPU *cpu, Reg8 reg) {
    uint8_t op = read_r8(cpu, reg);

    set_flags_addition(cpu, cpu->a, op);
    cpu->a += op;
}

void add_a_n8(CPU *cpu, uint8_t val) {
    set_flags_addition(cpu, cpu->a, val);
    cpu->a += val;
}

void cp_a_r8(CPU *cpu, Reg8 reg) {
    set_flags_subtraction(cpu, cpu->a, read_r8(cpu, reg));
}

void cp_a_n8(CPU *cpu, uint8_t val) {
    set_flags_subtraction(cpu, cpu->a, val);
}

void sbc_a_r8(CPU *cpu, Reg8 reg) {
    uint8_t op = read_r8(cpu, reg) + get_flag(cpu, CPU_FLAG_C);

    set_flags_subtraction(cpu, cpu->a, op);
    cpu->a -= op;
}

void sbc_a_n8(CPU *cpu, uint8_t val) {
    uint8_t op = val + get_flag(cpu, CPU_FLAG_C);

    set_flags_subtraction(cpu, cpu->a, op);
    cpu->a -= op;
}

void sub_a_r8(CPU *cpu, Reg8 reg) {
    uint8_t op = read_r8(cpu, reg);

    set_flags_subtraction(cpu, cpu->a, op);
    cpu->a -= op;
}

void sub_a_n8(CPU *cpu, uint8_t val) {
    set_flags_subtraction(cpu, cpu->a, val);
    cpu->a -= val;
}

void dec_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);

    write_r8(cpu, reg, val - 1);

    cpu->f = (cpu->f & 0x10)
        | ((uint8_t)(val - 1) == 0x00) << CPU_FLAG_Z
        | 0x40
        | (0x01 > val) << CPU_FLAG_H;
}

void inc_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);

    write_r8(cpu, reg, val + 1);

    cpu->f = (cpu->f & 0x10)
        | ((uint8_t)(val + 1) == 0x00) << CPU_FLAG_Z
        | ((val & 0x0F) + 1 > 0x0F) << CPU_FLAG_H;
}

void dec_r16(CPU *cpu, Reg16 reg) {
    write_r16(cpu, reg, read_r16(cpu, reg) - 1);
}

void inc_r16(CPU *cpu, Reg16 reg) {
    write_r16(cpu, reg, read_r16(cpu, reg) + 1);
}



// ------------------------------
//      bitwise instructions
// ------------------------------

void and_a_r8(CPU *cpu, Reg8 reg) {
    cpu->a &= read_r8(cpu, reg);
    set_flags_and(cpu, cpu->a);
}

void and_a_n8(CPU *cpu, uint8_t val) {
    cpu->a &= val;
    set_flags_and(cpu, cpu->a);
}

void or_a_r8(CPU *cpu, Reg8 reg) {
    cpu->a |= read_r8(cpu, reg);
    set_flags_or_xor(cpu, cpu->a);
}

void or_a_n8(CPU *cpu, uint8_t val) {
    cpu->a |= val; 
    set_flags_or_xor(cpu, cpu->a);
}

void xor_a_r8(CPU *cpu, Reg8 reg) {
    cpu->a ^= read_r8(cpu, reg);
    set_flags_or_xor(cpu, cpu->a);
}

void xor_a_n8(CPU *cpu, uint8_t val) {
    cpu->a ^= val;
    set_flags_or_xor(cpu, cpu->a);
}

void rlca(CPU *cpu) {
    uint8_t carry = (cpu->a >> 7) & 0x01;

    cpu->a = (cpu->a << 1) | carry;
    set_flags_roll_a(cpu, carry);
}

void rla(CPU *cpu) {
    uint8_t carry = (cpu->a >> 7) & 0x01;

    cpu->a = (cpu->a << 1) | get_flag(cpu, CPU_FLAG_C);
    set_flags_roll_a(cpu, carry);
}

void rrca(CPU *cpu) {
    uint8_t carry = cpu->a & 0x01;

    cpu->a = (cpu->a >> 1) | (carry << 7);
    set_flags_roll_a(cpu, carry);
}

void rra(CPU *cpu) {
    uint8_t carry = cpu->a & 0x01;

    cpu->a = (cpu->a >> 1) | (get_flag(cpu, CPU_FLAG_C) << 7);
    set_flags_roll_a(cpu, carry);
}

void cpl(CPU *cpu) {
    cpu->a = ~cpu->a;
    cpu->f |= 0x60;
}



// ---------------------------------
//      carry flag instructions
// ---------------------------------

void ccf(CPU *cpu) {
    cpu->f = (cpu->f & 0x80) | (~get_flag(cpu, CPU_FLAG_C) << CPU_FLAG_C);
}

void scf(CPU *cpu) {
    cpu->f = (cpu->f & 0x80) | 0x10;
}



// ----------------------------------------
//      interrupt-related instructions
// ----------------------------------------

void di(CPU *cpu) {
    cpu->ime = 0;
}

void ei(CPU *cpu) {
    cpu->ime_set_pending = 2;
}

void halt(CPU *cpu) {
    cpu->halted = 1;
}

void stop(CPU *cpu, uint8_t val) {
    // no idea how to implement this atm
}



// --------------------------------------------
//      jump, call and return instructions
// --------------------------------------------

void jr_e8(CPU *cpu, uint8_t val) {
    cpu->pc += (int8_t)val;
}

uint8_t jr_cond_e8(CPU *cpu, CPUCondition cond, uint8_t val) {
    if (evaluate_condition(cpu, cond) == 0) return 0;

    cpu->pc += (int8_t)val;
    return 1;
}

void jp_hl(CPU *cpu) {
    cpu->pc = read_r16(cpu, REG16_HL);
}

void jp_a16(CPU *cpu, uint16_t addr) {
    cpu->pc = addr;
}

uint8_t jp_cond_a16(CPU *cpu, CPUCondition cond, uint16_t addr) {
    if (evaluate_condition(cpu, cond) == 0) return 0;

    cpu->pc = addr;
    return 1;
}

void call_a16(CPU *cpu, uint16_t addr) {
    push_stack(cpu, cpu->pc);
    cpu->pc = addr;
}

uint8_t call_cond_a16(CPU *cpu, CPUCondition cond, uint16_t addr) {
    if (evaluate_condition(cpu, cond) == 0) return 0;

    push_stack(cpu, cpu->pc);
    cpu->pc = addr;
    return 1;
}

void ret(CPU *cpu) {
    cpu->pc = pop_stack(cpu);
}

uint8_t ret_cond(CPU *cpu, CPUCondition cond) {
    if (evaluate_condition(cpu, cond) == 0) return 0;

    cpu->pc = pop_stack(cpu);
    return 1;
}

void reti(CPU *cpu) {
    cpu->pc = pop_stack(cpu);
    cpu->ime = 1;
}

void rst_vec(CPU *cpu, uint8_t vec) {
    push_stack(cpu, cpu->pc);
    cpu->pc = vec;
}



// ---------------------------
//      misc instructions
// ---------------------------

void nop(CPU *cpu) {
    // does nothing
    return;
}

void daa(CPU *cpu) {
    uint8_t h = get_flag(cpu, CPU_FLAG_H);
    uint8_t c = get_flag(cpu, CPU_FLAG_C);

    if (get_flag(cpu, CPU_FLAG_N) == 1) {
        if (h == 1) cpu->a -= 0x06;
        if (c == 1) cpu->a -= 0x60;
    }
    else {
        if (h == 1 || (cpu->a & 0x0F) > 0x09) cpu->a += 0x06;
        if (c == 1 || cpu->a > 0x99) {
            cpu->a += 0x60;
            cpu->f |= 0x10;
        }
    }

    cpu->f |= (cpu->a == 0x00) << CPU_FLAG_Z;
}

void pop_r16(CPU *cpu, Reg16 dest) {
    write_r16(cpu, dest, pop_stack(cpu));
}

void push_r16(CPU *cpu, Reg16 src) {
    push_stack(cpu, read_r16(cpu, src));
}


// -------------------------------
//      prefixed instructions
// -------------------------------

void rlc_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);
    uint8_t carry = (val >> 7) & 0x01;
    uint8_t res = (val << 1) | carry;

    write_r8(cpu, reg, res);
    set_flags_roll_shift(cpu, carry, res);
}

void rrc_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);
    uint8_t carry = val & 0x01;
    uint8_t res = (val >> 1) | (carry << 7);

    write_r8(cpu, reg, res);
    set_flags_roll_shift(cpu, carry, res);
}

void rl_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);
    uint8_t carry = (val >> 7) & 0x01;
    uint8_t res = (val << 1) | get_flag(cpu, CPU_FLAG_C);

    write_r8(cpu, reg, res);
    set_flags_roll_shift(cpu, carry, res);
}

void rr_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);
    uint8_t carry = val & 0x01;
    uint8_t res = (val >> 1) | (get_flag(cpu, CPU_FLAG_C) << 7);

    write_r8(cpu, reg, res);
    set_flags_roll_shift(cpu, carry, res);
}

void sla_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);
    uint8_t res = val << 1;

    write_r8(cpu, reg, res);
    set_flags_roll_shift(cpu, (val >> 7) & 0x01, res);
}

void sra_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);
    uint8_t res = (val >> 1) | (val & 0x80);

    write_r8(cpu, reg, res);
    set_flags_roll_shift(cpu, val & 0x01, res);
}

void srl_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);
    uint8_t res = val >> 1;

    write_r8(cpu, reg, res);
    set_flags_roll_shift(cpu, val & 0x01, res);
}

void swap_r8(CPU *cpu, Reg8 reg) {
    uint8_t val = read_r8(cpu, reg);

    write_r8(cpu, reg, (val >> 4) | (val << 4));
    cpu->f |= (val == 0) << CPU_FLAG_Z;
}

void bit_r8(CPU *cpu, uint8_t bit, Reg8 reg) {
    cpu->f = (cpu->f & 0x10)
        | 0x20
        | ((((read_r8(cpu, reg) >> bit) & 0x01) == 0x00) << CPU_FLAG_Z);
}

void res_r8(CPU *cpu, uint8_t bit, Reg8 reg) {
    write_r8(cpu, reg, read_r8(cpu, reg) & ~(0x01 << bit));
}

void set_r8(CPU *cpu, uint8_t bit, Reg8 reg) {
    write_r8(cpu, reg, read_r8(cpu, reg) | (0x01 << bit));
}
