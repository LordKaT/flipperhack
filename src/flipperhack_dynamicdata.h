#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
    Dynamic Layout (32 bits):

    hp:     bits 0-7   (8)
    sp:     bits 8-15  (8)
    x:      bits 16-20 (5)
    y:      bits 21-25 (5)
    state:  bits 26-27 (2)
    fx:     bits 28-31 (4)
*/

#define DYNAMIC_HP       0
#define DYNAMIC_SP       1
#define DYNAMIC_X        2
#define DYNAMIC_Y        3
#define DYNAMIC_STATE    4
#define DYNAMIC_IN_FOV   5
#define DYNAMIC_FX       6

#define DYNAMIC_HP_BITS     8u
#define DYNAMIC_SP_BITS     8u
#define DYNAMIC_X_BITS      5u
#define DYNAMIC_Y_BITS      5u
#define DYNAMIC_STATE_BITS  2u
#define DYNAMIC_IN_FOV_BITS 1u
#define DYNAMIC_FX_BITS     4u

#define DYNAMIC_HP_MASK     ((1u << DYNAMIC_HP_BITS)     - 1u)     // 0xFF
#define DYNAMIC_SP_MASK     ((1u << DYNAMIC_SP_BITS)     - 1u)     // 0xFF
#define DYNAMIC_X_MASK      ((1u << DYNAMIC_X_BITS)      - 1u)     // 0x1F
#define DYNAMIC_Y_MASK      ((1u << DYNAMIC_Y_BITS)      - 1u)     // 0x1F
#define DYNAMIC_STATE_MASK  ((1u << DYNAMIC_STATE_BITS)  - 1u)     // 0x03
#define DYNAMIC_IN_FOV_MASK ((1u << DYNAMIC_IN_FOV_BITS) - 1u)     // 0x01
#define DYNAMIC_FX_MASK     ((1u << DYNAMIC_FX_BITS)     - 1u)     // 0x0F

#define DYNAMIC_HP_SHIFT        0u
#define DYNAMIC_SP_SHIFT        8u
#define DYNAMIC_X_SHIFT         16u
#define DYNAMIC_Y_SHIFT         21u
#define DYNAMIC_STATE_SHIFT     26u
#define DYNAMIC_IN_FOV_SHIFT    27u
#define DYNAMIC_FX_SHIFT        28u

static inline uint32_t dynamicdata_pack(
    uint8_t hp,
    uint8_t sp,
    uint8_t x,
    uint8_t y,
    uint8_t state,
    bool in_fov, // actually we don't need this, because we can check the map visibility.
    uint8_t fx
){
    uint32_t v = 0;

    v |= ((uint32_t)(hp    & DYNAMIC_HP_MASK)    << DYNAMIC_HP_SHIFT);
    v |= ((uint32_t)(sp    & DYNAMIC_SP_MASK)    << DYNAMIC_SP_SHIFT);
    v |= ((uint32_t)(x     & DYNAMIC_X_MASK)     << DYNAMIC_X_SHIFT);
    v |= ((uint32_t)(y     & DYNAMIC_Y_MASK)     << DYNAMIC_Y_SHIFT);
    v |= ((uint32_t)(state & DYNAMIC_STATE_MASK) << DYNAMIC_STATE_SHIFT);
    v |= ((uint32_t)(in_fov & DYNAMIC_IN_FOV_MASK) << DYNAMIC_IN_FOV_SHIFT);
    v |= ((uint32_t)(fx    & DYNAMIC_FX_MASK)    << DYNAMIC_FX_SHIFT);

    return v;
}

static inline uint32_t dynamicdata_get(uint32_t packed, uint8_t field) {
    switch (field) {
        case DYNAMIC_HP:
            return (packed >> DYNAMIC_HP_SHIFT) & DYNAMIC_HP_MASK;
        case DYNAMIC_SP:
            return (packed >> DYNAMIC_SP_SHIFT) & DYNAMIC_SP_MASK;
        case DYNAMIC_X:
            return (packed >> DYNAMIC_X_SHIFT) & DYNAMIC_X_MASK;
        case DYNAMIC_Y:
            return (packed >> DYNAMIC_Y_SHIFT) & DYNAMIC_Y_MASK;
        case DYNAMIC_STATE:
            return (packed >> DYNAMIC_STATE_SHIFT) & DYNAMIC_STATE_MASK;
        case DYNAMIC_IN_FOV:
            return (packed >> DYNAMIC_IN_FOV_SHIFT) & DYNAMIC_IN_FOV_MASK;
        case DYNAMIC_FX:
            return (packed >> DYNAMIC_FX_SHIFT) & DYNAMIC_FX_MASK;
        default:
            return 0;
    }
}

static inline void dynamicdata_set(uint32_t* packed, uint8_t field, uint32_t value) {
    uint32_t mask, shift;

    switch (field) {
        case DYNAMIC_HP:
            mask = DYNAMIC_HP_MASK;
            shift = DYNAMIC_HP_SHIFT;
            break;
        case DYNAMIC_SP:
            mask = DYNAMIC_SP_MASK;
            shift = DYNAMIC_SP_SHIFT;
            break;
        case DYNAMIC_X:
            mask = DYNAMIC_X_MASK;
            shift = DYNAMIC_X_SHIFT;
            break;
        case DYNAMIC_Y:
            mask = DYNAMIC_Y_MASK;
            shift = DYNAMIC_Y_SHIFT;
            break;
        case DYNAMIC_STATE:
            mask = DYNAMIC_STATE_MASK;
            shift = DYNAMIC_STATE_SHIFT;
            break;
        case DYNAMIC_IN_FOV:
            mask = DYNAMIC_IN_FOV_MASK;
            shift = DYNAMIC_IN_FOV_SHIFT;
            break;
        case DYNAMIC_FX:
            mask = DYNAMIC_FX_MASK;
            shift = DYNAMIC_FX_SHIFT;
            break;
        default:
            return;
    }

    *packed = (*packed & ~(mask << shift)) | ((value & mask) << shift);
}

static inline uint8_t dynamicdata_get_hp(uint32_t p) {
    return (p >> DYNAMIC_HP_SHIFT) & DYNAMIC_HP_MASK;
}

static inline void dynamicdata_set_hp(uint32_t* p, uint8_t v) {
    *p = (*p & ~(DYNAMIC_HP_MASK << DYNAMIC_HP_SHIFT)) |
         ((v & DYNAMIC_HP_MASK) << DYNAMIC_HP_SHIFT);
}

static inline uint8_t dynamicdata_get_sp(uint32_t p) {
    return (p >> DYNAMIC_SP_SHIFT) & DYNAMIC_SP_MASK;
}

static inline void dynamicdata_set_sp(uint32_t* p, uint8_t v) {
    *p = (*p & ~(DYNAMIC_SP_MASK << DYNAMIC_SP_SHIFT)) |
         ((v & DYNAMIC_SP_MASK) << DYNAMIC_SP_SHIFT);
}

static inline uint8_t dynamicdata_get_x(uint32_t p) {
    return (p >> DYNAMIC_X_SHIFT) & DYNAMIC_X_MASK;
}

static inline void dynamicdata_set_x(uint32_t* p, uint8_t v) {
    *p = (*p & ~(DYNAMIC_X_MASK << DYNAMIC_X_SHIFT)) |
         ((v & DYNAMIC_X_MASK) << DYNAMIC_X_SHIFT);
}

static inline uint8_t dynamicdata_get_y(uint32_t p) {
    return (p >> DYNAMIC_Y_SHIFT) & DYNAMIC_Y_MASK;
}

static inline void dynamicdata_set_y(uint32_t* p, uint8_t v) {
    *p = (*p & ~(DYNAMIC_Y_MASK << DYNAMIC_Y_SHIFT)) |
         ((v & DYNAMIC_Y_MASK) << DYNAMIC_Y_SHIFT);
}

static inline uint8_t dynamicdata_get_state(uint32_t p) {
    return (p >> DYNAMIC_STATE_SHIFT) & DYNAMIC_STATE_MASK;
}

static inline void dynamicdata_set_state(uint32_t* p, uint8_t v) {
    *p = (*p & ~(DYNAMIC_STATE_MASK << DYNAMIC_STATE_SHIFT)) |
         ((v & DYNAMIC_STATE_MASK) << DYNAMIC_STATE_SHIFT);
}

static inline bool dynamicdata_get_in_fov(uint32_t p) {
    return (p >> DYNAMIC_IN_FOV_SHIFT) & DYNAMIC_IN_FOV_MASK;
}

static inline void dynamicdata_set_in_fov(uint32_t* p, bool v) {
    *p = (*p & ~(DYNAMIC_IN_FOV_MASK << DYNAMIC_IN_FOV_SHIFT)) |
         ((v & DYNAMIC_IN_FOV_MASK) << DYNAMIC_IN_FOV_SHIFT);
}

static inline uint8_t dynamicdata_get_fx(uint32_t p) {
    return (p >> DYNAMIC_FX_SHIFT) & DYNAMIC_FX_MASK;
}

static inline void dynamicdata_set_fx(uint32_t* p, uint8_t v) {
    *p = (*p & ~(DYNAMIC_FX_MASK << DYNAMIC_FX_SHIFT)) |
         ((v & DYNAMIC_FX_MASK) << DYNAMIC_FX_SHIFT);
}
