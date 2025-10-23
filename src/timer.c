#include "timer.h"
#include "gb.h"

static uint16_t get_clock_inc_cycles(TimerClock clock) {
    uint16_t val = 0;

    switch(clock) {
        case TIMER_CLOCK_256:
            val = 256; break;
        case TIMER_CLOCK_4:
            val = 4; break;
        case TIMER_CLOCK_16:
            val = 16; break;
        case TIMER_CLOCK_64:
            val = 64; break;
    }

    return val;
}


void timer_init(Timer *timer, GB *gb) {
    timer->timer_counter  = 0;
    timer->divider_counter = 0;
    timer->gb = gb;
}

void timer_step(Timer *timer, uint8_t cycles) {
    uint8_t tac = timer->gb->io[TAC_ADDR_RELATIVE];

    uint8_t tima = timer->gb->io[TIMA_ADDR_RELATIVE];

    for (uint8_t c = 0; c < cycles; c++) {
        timer->divider_counter++;

        if (timer->divider_counter == DIV_INC_CYCLES) {
            timer->gb->io[DIV_ADDR_RELATIVE]++;
            timer->divider_counter = 0;
        }
        
        if (!(tac & TAC_ENABLE_MASK)) continue;

        timer->timer_counter++;

        if (timer->timer_counter == get_clock_inc_cycles(tac & TAC_CLOCK_SELECT_MASK)) {
            if (tima == 0xFF) {
                tima = timer->gb->io[TMA_ADDR_RELATIVE];;
                gb_interrupt(timer->gb, INTERRUPT_TIMER);
            }
            else
                tima++;

            timer->gb->io[TIMA_ADDR_RELATIVE] = tima;
        }

        
    }
}

void timer_div_reset(Timer *timer) {
    timer->gb->io[DIV_ADDR_RELATIVE] = 0x00;
}