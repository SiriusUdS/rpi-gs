#pragma once

#include <stdint.h>

#define VALVE_STATE_UNKNOWN (uint8_t)0x00
#define VALVE_STATE_OPENED  (uint8_t)0x01
#define VALVE_STATE_CLOSED  (uint8_t)0x02
#define VALVE_STATE_OPENING (uint8_t)0x03
#define VALVE_STATE_CLOSING (uint8_t)0x04