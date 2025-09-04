#ifndef CPU_H
#define CPU_H

#include <stdint.h>

// NOTE:
// The [hl] operand is treated like a register here
// because it is encoded in opcodes as if it was one.
// Keep in mind that 'r8' in function names also refers to [hl]
typedef enum Reg8 {
    REG8_B = 0,
    REG8_C = 1,
    REG8_D = 2,
    REG8_E = 3,
    REG8_H = 4,
    REG8_L = 5,
    REG8_HLMEM = 6,
    REG8_A = 7
} Reg8;

typedef enum Reg16 {
    REG16_BC = 0,
    REG16_DE = 1,
    REG16_HL = 2,
    REG16_SP = 3,
    REG16_AF = 4
} Reg16;

typedef enum Condition {
    COND_NZ = 0,
    COND_Z = 1,
    COND_NC = 2,
    COND_C = 3
} Condition;

typedef enum Flag {
    FLAG_Z = 7,
    FLAG_N = 6,
    FLAG_H = 5,
    FLAG_C = 4
} Flag;

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
    uint8_t halted;

    struct Gameboy *gameboy;
} CPU;

void cpu_init(CPU *cpu, struct Gameboy *gb);

uint8_t cpu_step(CPU *cpu);

uint8_t cpu_execute(CPU *cpu, uint8_t opcode);
uint8_t cpu_execute_prefixed(CPU *cpu, uint8_t opcode);

// helper functions

uint8_t read_r8(CPU *cpu, Reg8 reg);
uint16_t read_r16(CPU *cpu, Reg16 reg);

void write_r8(CPU *cpu, Reg8 reg, uint8_t val);
void write_r16(CPU *cpu, Reg16 reg, uint16_t val);

void push_stack(CPU *cpu, uint16_t val);
uint16_t pop_stack(CPU *cpu);

// load instructions

void ld_r8_r8(CPU *cpu, Reg8 dest, Reg8 src);
void ld_r8_n8(CPU *cpu, Reg8 dest, uint8_t val);
void ld_r16_n16(CPU *cpu, Reg16 dest, uint16_t val);

void ld_r16mem_a(CPU *cpu, Reg16 dest);
void ld_a16_a(CPU *cpu, uint16_t dest);
void ld_a_r16mem(CPU *cpu, Reg16 src);
void ld_a_a16(CPU *cpu, uint16_t src);

void ldh_a8_a(CPU *cpu, uint8_t dest);
void ldh_cmem_a(CPU *cpu);
void ldh_a_a8(CPU *cpu, uint8_t src);
void ldh_a_cmem(CPU *cpu);

void ldi_hlmem_a(CPU *cpu);
void ldd_hlmem_a(CPU *cpu);
void ldi_a_hlmem(CPU *cpu);
void ldd_a_hlmem(CPU *cpu);

void ld_sp_n16(CPU *cpu, uint16_t val);
void ld_a16_sp(CPU *cpu, uint16_t dest);
void ld_hl_sp_e8(CPU *cpu, uint8_t val);
void ld_sp_hl(CPU *cpu);

// arithmetic instructions

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

// bitwise instructions

void and_a_r8(CPU *cpu, Reg8 reg);
void and_a_n8(CPU *cpu, uint8_t val);

void or_a_r8(CPU *cpu, Reg8 reg);
void or_a_n8(CPU *cpu, uint8_t val);

void xor_a_r8(CPU *cpu, Reg8 reg);
void xor_a_n8(CPU *cpu, uint8_t val);

void rlca(CPU *cpu);
void rla(CPU *cpu);

void rrca(CPU *cpu);
void rra(CPU *cpu);

void cpl(CPU *cpu);

// carry instructions

void ccf(CPU *cpu);
void scf(CPU *cpu);

// interrupt-related instructions

void di(CPU *cpu);
void ei(CPU *cpu);

void halt(CPU *cpu);
void stop(CPU *cpu, uint8_t val);

// jump, call and return instructions

void jr_e8(CPU *cpu, uint8_t val);
uint8_t jr_cond_e8(CPU *cpu, Condition cond, uint8_t val);

void jp_hl(CPU *cpu);
void jp_a16(CPU *cpu, uint16_t addr);
uint8_t jp_cond_a16(CPU *cpu, Condition cond, uint16_t addr);

void call_a16(CPU *cpu, uint16_t addr);
uint8_t call_cond_a16(CPU *cpu, Condition cond, uint16_t addr);

void ret(CPU *cpu);
uint8_t ret_cond(CPU *cpu, Condition cond);
void reti(CPU *cpu);

void rst_vec(CPU *cpu, uint8_t vec);

// misc instructions

void nop(CPU *cpu);

void daa(CPU *cpu);

void pop_r16(CPU *cpu, Reg16 dest);

void push_r16(CPU *cpu, Reg16 src);

// prefixed instructions

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
