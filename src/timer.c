#include "timer.h"
#include "gb.h"

void timer_step(Timer *timer, uint8_t cycles) {
    uint8_t tac = timer->gb->io[TAC_ADDR_RELATIVE];

    if (!(tac & TAC_ENABLE_MASK)) return;

    uint8_t tma = timer->gb->io[TMA_ADDR_RELATIVE];
    uint8_t tima = timer->gb->io[TIMA_ADDR_RELATIVE];

    
}