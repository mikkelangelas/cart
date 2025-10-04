#include "joypad.h"

#include "gb.h"

void joypad_init(Joypad *joypad, GB *gb) {
    joypad_reset(joypad);
    joypad->gb = gb;
}

void joypad_reset(Joypad *joypad) {
    // mark all buttons as released
    joypad->buttons = 0x0F;
    joypad->dpad = 0x0F;
}

void joypad_press(Joypad *joypad, JoypadButton button) {
    gb_interrupt(joypad->gb, INTERRUPT_JOYPAD);

    if (button < JOYPAD_BUTTON_RIGHT) joypad->buttons &= ~(1 << button);
    else joypad->dpad &= ~(1 << (button - JOYPAD_BUTTON_RIGHT));
}

void joypad_update(Joypad *joypad) {
    uint8_t joyp = joypad->gb->io[JOYP_ADDR - IO_BASE_ADDR] & JOYP_SELECT_MASK;
    
    switch (joyp) {
        case JOYP_SELECT_BUTTONS:
            joyp |= joypad->buttons; break;
        case JOYP_SELECT_DPAD:
            joyp |= joypad->dpad; break;
        default:
            joyp |= 0x0F;
    }

    joypad->gb->io[JOYP_ADDR - IO_BASE_ADDR] = joyp;
}
