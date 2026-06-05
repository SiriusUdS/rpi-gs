#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t state : 6;
    uint16_t RESERVED : 10;
  }
  bits;
  uint16_t value;
}
FillingStationStatus;