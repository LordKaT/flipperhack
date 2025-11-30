// yeah it should be called "infrequentlyaccesseddata" but too long so I make small.
#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
    Static Layout (16 bits?):

    hp_max:     bits 0-7    (8)
    sp_max:     bits 8-15   (8)
*/

#define STATIC_HP_MAX   0
#define STATIC_SP_MAX   1

#define STATIC_HP_MAX_BITS 8u
#define STATIC_SP_MAX_BITS 8u

#define STATIC_HP_MAX_MASK  ((1u << STATIC_HP_MAX_BITS) - 1u)
#define STATIC_SP_MAX_MASK  ((1u << STATIC_SP_MAX_BITS) - 1u)

#define STATIC_HP_MAX_SHIFT 0u
#define STATIC_SP_MAX_SHIFT 8u

static inline uint16_t staticdata_pack(
    uint8_t hp_max,
    uint8_t sp_max
){
    uint16_t v = 0;

    v |= ((uint16_t)(hp_max & STATIC_HP_MAX_MASK) << STATIC_HP_MAX_SHIFT);
    v |= ((uint16_t)(sp_max & STATIC_SP_MAX_MASK) << STATIC_SP_MAX_SHIFT);

    return v;
}

static inline uint16_t staticdata_get(uint16_t packed, uint8_t field) {
    switch (field) {
        case STATIC_HP_MAX:
            return (packed >> STATIC_HP_MAX_SHIFT) & STATIC_HP_MAX_MASK;
        case STATIC_SP_MAX:
            return (packed >> STATIC_SP_MAX_SHIFT) & STATIC_SP_MAX_MASK;
        default:
            return 0;
    }
}

static inline void staticdata_set(uint16_t* packed, uint8_t field, uint8_t value) {
    uint16_t mask, shift;

    switch (field) {
        case STATIC_HP_MAX:
            mask = STATIC_HP_MAX_MASK;
            shift = STATIC_HP_MAX_SHIFT;
            break;
        case STATIC_SP_MAX:
            mask = STATIC_SP_MAX_MASK;
            shift = STATIC_SP_MAX_SHIFT;
            break;
        default:
            return;
    }

    *packed = (*packed & ~(mask << shift)) | ((value & mask) << shift);
}

static inline uint8_t staticdata_get_hp_max(uint16_t p) {
    return (p >> STATIC_HP_MAX_SHIFT) & STATIC_HP_MAX_MASK;
}

static inline void staticdata_set_hp_max(uint16_t* p, uint8_t v) {
    *p = (*p & ~(STATIC_HP_MAX_MASK << STATIC_HP_MAX_SHIFT)) |
         ((v & STATIC_HP_MAX_MASK) << STATIC_HP_MAX_SHIFT);
}

static inline uint8_t staticdata_get_sp_max(uint16_t p) {
    return (p >> STATIC_SP_MAX_SHIFT) & STATIC_SP_MAX_MASK;
}

static inline void staticdata_set_sp_max(uint16_t* p, uint8_t v) {
    *p = (*p & ~(STATIC_SP_MAX_MASK << STATIC_SP_MAX_SHIFT)) |
         ((v & STATIC_SP_MAX_MASK) << STATIC_SP_MAX_SHIFT);
}

/* Scratchpad

Idea: this data is the less changed data.
For enemies, this will live on the SD card (ROM file)
For player this will be used to calculate new stats based on levels and equipment.

This data should not be updated frequently. Maybe "staticdata" is the wrong name
but "infrequentlyuseddata" sucks.


Data player needs:
hp_max      8 bits
sp_max      8 bits
level       5 bits
is_dead     -- not needed, if dead game state set to dead

total bits: 21


Data enemies need:
hp_max      8 bits
sp_max      8 bits
level       -- not needed, only for loading maps
is_dead     -- not needed, if dead becomes corpse item
glyph       8 bits (6 bits if baudot?)
id          8 bits (0-254)

total       33 bits (31 bits if baudot?)

shared:
hp_max      8 bits
sp_max      8 bits

total       16 bits

player w/o shared:
level       5 bits

enemy w/o shared:
glyph       8 bits (6 bit if baudot?)
id          8 bits (0-254)


But wait, what data do we NEED to store for enemies?

enemies do not gain levels or xp.

Static, but needs accibility in some way:
id: Needs to be stored to identify what data to fetch

Static, can be pulled from SD card without changing:
glyph
hp_max
sp_max
stats
loot table

Dynamic, needs to be updated during gameplay:
hp      8 bits
sp      8 bits
x       5 bits
y       5 bits
state   2 bits (see below)
spare   4 bits

        32 bits


flags:

state, 2 bits: 0-3:
    0: is player
    1: idle
    2: in FOV, active hunt
    3: flee
3
4
5
6

*/