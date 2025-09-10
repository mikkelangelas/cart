#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdint.h>

#define JOYP_ADDR 0xFF00

#define JOYP_SELECT_MASK 0x30
#define JOYP_SELECT_BUTTONS 0x10
#define JOYP_SELECT_DPAD 0x20

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
    uint8_t buttons;
    uint8_t dpad;

    struct GB *gb;
} Joypad;

void joypad_init(Joypad *joypad, struct GB *gb);

void joypad_reset(Joypad *joypad);

void joypad_press(Joypad *joypad, JoypadButton button);

void joypad_update(Joypad *joypad);

#endif
