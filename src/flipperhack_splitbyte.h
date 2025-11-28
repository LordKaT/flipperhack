#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SPLITBYTE_BITS 4u
#define SPLITBYTE_MASK ((uint8_t)((1u << SPLITBYTE_BITS) - 1u))

#define SPLITBYTE_LOW 0
#define SPLITBYTE_HIGH 1

#define SPLITBYTE_MODE SPLITBYTE_LOW
#define SPLITBYTE_ENEMY SPLITBYTE_HIGH

static inline uint8_t splitbyte_pack(uint8_t high, uint8_t low) {
    return (high << SPLITBYTE_BITS) | low;
}

static inline uint8_t splitbyte_set_low(uint8_t splitbyte, uint8_t low) {
    // Mask 'low' so it cannot bleed into the high nibble.
    low &= SPLITBYTE_MASK;

    return (splitbyte & (SPLITBYTE_MASK << SPLITBYTE_BITS)) | low;
}

static inline uint8_t splitbyte_set_high(uint8_t splitbyte, uint8_t high) {
    // Mask 'high' for safety as well.
    high &= SPLITBYTE_MASK;

    return (high << SPLITBYTE_BITS) | (splitbyte & SPLITBYTE_MASK);
}

static inline uint8_t splitbyte_get(uint8_t splitbyte, uint8_t index) {
    return (splitbyte >> (index * SPLITBYTE_BITS)) & SPLITBYTE_MASK;
}

static inline uint8_t splitbyte_get_high(uint8_t splitbyte) {
    return splitbyte >> SPLITBYTE_BITS;
}

static inline uint8_t splitbyte_get_low(uint8_t splitbyte) {
    return splitbyte & SPLITBYTE_MASK;
}
