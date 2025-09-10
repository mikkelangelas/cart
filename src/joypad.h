#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdint.h>

typedef enum JoypadButton {
    JOYPAD_BUTTON_A = 0,
    JOYPAD_BUTTON_B = 1,
    JOYPAD_BUTTON_SELECT = 2,
    JOYPAD_BUTTON_START = 3,
    JOYPAD_BUTTON_RIGHT = 4,
    JOYPAD_BUTTON_LEFT = 5,
    JOYPAD_BUTTON_UP = 6,
    JOYPAD_BUTTON_DOWN = 7
} JoypadButton;

typedef struct Joypad {
    struct GB *gb;
    uint8_t buttons;
    uint8_t dpad;
} Joypad;

void joypad_press(Joypad *joypad, JoypadButton button);

#endif
