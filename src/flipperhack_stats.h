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

extern uint32_t stats_pack(
    uint8_t str,
    uint8_t dex,
    uint8_t con,
    uint8_t intl,
    uint8_t wis,
    uint8_t cha,
    bool flag1,
    bool flag2
);
extern uint8_t stats_get(uint32_t packed, uint8_t stat);
extern void stats_set_flag(uint32_t* packed, uint32_t flag);
extern void stats_clear_flag(uint32_t* packed, uint32_t flag);
extern bool stats_get_flag(uint32_t packed, uint32_t flag);
