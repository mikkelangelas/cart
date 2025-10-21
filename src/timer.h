#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define DIV_INC_CYCLES 64

// addresses relative to the start of IO memory
#define DIV_ADDR_RELATIVE 0x0004
#define TIMA_ADDR_RELATIVE 0x0005
#define TMA_ADDR_RELATIVE 0x0006
#define TAC_ADDR_RELATIVE 0x0007

#define TAC_ENABLE_MASK 0x04
#define TAC_CLOCK_SELECT_MASK 0x03

typedef enum TimerClock {
    TIMER_CLOCK_256 = 0,
    TIMER_CLOCK_4 = 1,
    TIMER_CLOCK_16 = 2,
    TIMER_CLOCK_64 = 3
} TimerClock;

typedef struct Timer {
    uint8_t cycle_counter;
    struct GB *gb;
} Timer;

void timer_step(Timer *timer, uint8_t cycles);

#endif