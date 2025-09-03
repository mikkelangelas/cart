#ifndef CPU_H
#define CPU_H

#include <stdint.h>

// NOTE:
// The [hl] operand is treated like a register here
// because it is encoded in opcodes as if it was one.
// Keep in mind that 'r8' in function names also refers to [hl]
typedef enum Reg8 {
    B = 0,
    C = 1,
    D = 2,
    E = 3,
    H = 4,
    L = 5,
    HLMEM = 6,
    A = 7
} Reg8;

typedef enum Reg16 {
    AF,
    BC,
    DE,
    HL,
    SP
} Reg16;

typedef struct CPU {
    uint8_t a;
    uint8_t f;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;

    uint16_t pc;
    uint16_t sp;

    uint8_t ime;
} CPU;

void cpu_init(CPU *cpu);

uint8_t cpu_step(CPU *cpu);

uint8_t cpu_execute(CPU *cpu, uint8_t opcode);
uint8_t cpu_execute_prefixed(CPU *cpu, uint8_t opcode);

uint8_t read_r8(CPU *cpu, Reg8 reg);
uint16_t read_r16(CPU *cpu, Reg16 reg);

void write_r8(CPU *cpu, Reg8 reg);
void write_r16(CPU *cpu, Reg16 reg);

void nop(CPU *cpu);

void ld_r8_r8(CPU *cpu, Reg8 dest, Reg8 src);
void ld_r8_n8(CPU *cpu, Reg8 dest, uint8_t val);
void ld_r16_n16(CPU *cpu, Reg16 dest, uint16_t val);

void ld_r16mem_a(CPU *cpu, Reg16 dest);
void ld_n16mem_a(CPU *cpu, uint16_t dest);
void ld_a_r16mem(CPU *cpu, Reg16 src);
void ld_a_n16mem(CPU *cpu, uint16_t src);

void ldh_n8mem_a(CPU *cpu, uint8_t dest);
void ldh_cmem_a(CPU *cpu);
void ldh_a_n8mem(CPU *cpu, uint8_t src);
void ldh_a_cmem(CPU *cpu);

void ldi_hlmem_a(CPU *cpu);
void ldd_hlmem_a(CPU *cpu);
void ldi_a_hlmem(CPU *cpu);
void ldd_a_hlmem(CPU *cpu);

void ld_sp_n16(CPU *cpu, uint16_t val);
void ld_n16mem_sp(CPU *cpu, uint16_t dest);
void ld_hl_sp_e8(CPU *cpu, uint8_t val);
void ld_sp_hl(CPU *cpu);

// arithmetic operations

void add_sp_e8(CPU *cpu, uint8_t val);

void add_hl_r16(CPU *cpu, Reg16 reg);

void adc_a_r8(CPU *cpu, Reg8 reg);
void adc_a_n8(CPU *cpu, uint8_t val);

void add_a_r8(CPU *cpu, Reg8 reg);
void add_a_n8(CPU *cpu, uint8_t val);

void cp_a_r8(CPU *cpu, Reg8 reg);
void cp_a_n8(CPU *cpu, uint8_t val);

void sbc_a_r8(CPU *cpu, Reg8 reg);
void sbc_a_n8(CPU *cpu, uint8_t val);

void sub_a_r8(CPU *cpu, Reg8 reg);
void sub_a_n8(CPU *cpu, uint8_t val);

void dec_r8(CPU *cpu, Reg8 reg);
void inc_r8(CPU *cpu, Reg8 reg);

void dec_r16(CPU *cpu, Reg16 reg);
void inc_r16(CPU *cpu, Reg16 reg);

void cp_a_r8(CPU *cpu, Reg8 reg);
void cp_a_n8(CPU *cpu, uint8_t val);

// bitwise logic

void and_a_r8(CPU *cpu, Reg8 reg);
void and_a_n8(CPU *cpu, uint8_t val);

void or_a_r8(CPU *cpu, Reg8 reg);
void or_a_n8(CPU *cpu, uint8_t val);

void xor_a_r8(CPU *cpu, Reg8 reg);
void xor_a_n8(CPU *cpu, uint8_t val);

// prefixed operations

void rlc_r8(CPU *cpu, Reg8 reg);

void rrc_r8(CPU *cpu, Reg8 reg);

void rl_r8(CPU *cpu, Reg8 reg);

void rr_r8(CPU *cpu, Reg8 reg);

void sla_r8(CPU *cpu, Reg8 reg);

void sra_r8(CPU *cpu, Reg8 reg);

void swap_r8(CPU *cpu, Reg8 reg);

void srl_r8(CPU *cpu, Reg8 reg);

void bit_r8(CPU *cpu, uint8_t bit, Reg8 reg);

void res_r8(CPU *cpu, uint8_t bit, Reg8 reg);

void set_r8(CPU *cpu, uint8_t bit, Reg8 reg);


#endif
