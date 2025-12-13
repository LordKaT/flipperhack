#pragma once

#include <stdbool.h>
#include <stdint.h>

#define STATS_STR 0
#define STATS_DEX 1
#define STATS_CON 2
#define STATS_INT 3
#define STATS_WIS 4
#define STATS_CHA 5

#define STAT_BITS 5u
#define STAT_MASK ((uint32_t)((1u << STAT_BITS) - 1u))

#define STAT_FLAG1_BIT 30u
#define STAT_FLAG2_BIT 31u

#define STAT_FLAG1 ((uint32_t)1u << STAT_FLAG1_BIT)
#define STAT_FLAG2 ((uint32_t)1u << STAT_FLAG2_BIT)

static inline uint32_t stats_pack(
    uint8_t str,
    uint8_t dex,
    uint8_t con,
    uint8_t intl,
    uint8_t wis,
    uint8_t cha,
    bool flag1,
    bool flag2
) {
    uint32_t v = 0;
    v |= ((uint32_t)(str & STAT_MASK) << (STAT_BITS * STATS_STR));
    v |= ((uint32_t)(dex & STAT_MASK) << (STAT_BITS * STATS_DEX));
    v |= ((uint32_t)(con & STAT_MASK) << (STAT_BITS * STATS_CON));
    v |= ((uint32_t)(intl & STAT_MASK) << (STAT_BITS * STATS_INT));
    v |= ((uint32_t)(wis & STAT_MASK) << (STAT_BITS * STATS_WIS));
    v |= ((uint32_t)(cha & STAT_MASK) << (STAT_BITS * STATS_CHA));
    v |= (flag1 ? STAT_FLAG1 : 0);
    v |= (flag2 ? STAT_FLAG2 : 0);
    return v;
}

static inline uint8_t stats_get(uint32_t packed, uint8_t stat) {
    uint32_t shift = (uint32_t)stat * STAT_BITS;
    return (uint8_t)((packed >> shift) & STAT_MASK);
}

static inline void stats_set_flag(uint32_t* packed, uint32_t flag) {
    *packed |= flag;
}

static inline void stats_clear_flag(uint32_t* packed, uint32_t flag) {
    *packed &= ~flag;
}

static inline bool stats_get_flag(uint32_t packed, uint32_t flag) {
    return (packed & flag) != 0;
}
