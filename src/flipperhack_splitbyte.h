#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SPLITBYTE_BITS 4u
#define SPLITBYTE_MASK ((uint8_t)((1u << SPLITBYTE_BITS) - 1u))

#define SPLITBYTE_LOW 0
#define SPLITBYTE_HIGH 1

#define SPLITBYTE_MODE SPLITBYTE_LOW
#define SPLITBYTE_ENEMY SPLITBYTE_HIGH

extern uint8_t splitbyte_pack(uint8_t high, uint8_t low);
extern uint8_t splitbyte_set_high(uint8_t splitbyte, uint8_t high);
extern uint8_t splitbyte_set_low(uint8_t splitbyte, uint8_t low);
extern uint8_t splitbyte_get(uint8_t splitbyte, uint8_t index);
extern uint8_t splitbyte_get_high(uint8_t splitbyte);
extern uint8_t splitbyte_get_low(uint8_t splitbyte);
