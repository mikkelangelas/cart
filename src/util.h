#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

inline void set_bit(uint8_t *dest, uint8_t bit, uint8_t val) {
    if (val == 0) *dest &= ~(0x01 << bit);
    else *dest |= 0x01 << bit;
}

inline uint8_t get_bit(uint8_t src, uint8_t bit) {
    return (src >> bit) & 0x01;
}

uint8_t *read_file_to_array(const char *filename, uint8_t is_binary);

#endif
