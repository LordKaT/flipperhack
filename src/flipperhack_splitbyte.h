#pragma once

#include <stdbool.h>
#include <stdint.h>

// 5 bits for High (Enemy Count, max 31)
// 3 bits for Low (Game Mode, max 7)
#define SPLITBYTE_HIGH_BITS 5u
#define SPLITBYTE_LOW_BITS 3u

#define SPLITBYTE_HIGH_MASK ((uint8_t)((1u << SPLITBYTE_HIGH_BITS) - 1u))
#define SPLITBYTE_LOW_MASK ((uint8_t)((1u << SPLITBYTE_LOW_BITS) - 1u))

#define SPLITBYTE_LOW 0
#define SPLITBYTE_HIGH 1

#define SPLITBYTE_MODE SPLITBYTE_LOW
#define SPLITBYTE_ENEMY SPLITBYTE_HIGH

static inline uint8_t splitbyte_pack(uint8_t high, uint8_t low) {
    return ((high & SPLITBYTE_HIGH_MASK) << SPLITBYTE_LOW_BITS) | (low & SPLITBYTE_LOW_MASK);
}

static inline uint8_t splitbyte_set_low(uint8_t splitbyte, uint8_t low) {
    // Mask 'low' so it cannot bleed into the high nibble.
    low &= SPLITBYTE_LOW_MASK;

    return (splitbyte & (SPLITBYTE_HIGH_MASK << SPLITBYTE_LOW_BITS)) | low;
}

static inline uint8_t splitbyte_set_high(uint8_t splitbyte, uint8_t high) {
    // Mask 'high' for safety as well.
    high &= SPLITBYTE_HIGH_MASK;

    return (high << SPLITBYTE_LOW_BITS) | (splitbyte & SPLITBYTE_LOW_MASK);
}

static inline uint8_t splitbyte_get(uint8_t splitbyte, uint8_t index) {
    if (index == SPLITBYTE_LOW) {
        return splitbyte & SPLITBYTE_LOW_MASK;
    } else {
        return (splitbyte >> SPLITBYTE_LOW_BITS) & SPLITBYTE_HIGH_MASK;
    }
}

static inline uint8_t splitbyte_get_high(uint8_t splitbyte) {
    return (splitbyte >> SPLITBYTE_LOW_BITS) & SPLITBYTE_HIGH_MASK;
}

static inline uint8_t splitbyte_get_low(uint8_t splitbyte) {
    return splitbyte & SPLITBYTE_LOW_MASK;
}
