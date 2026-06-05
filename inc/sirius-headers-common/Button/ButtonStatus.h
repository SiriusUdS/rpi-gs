#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t ignited : 1;
    uint16_t isPressed : 1;
    uint16_t RESERVED : 14;
  }
  bits;

  uint16_t value;
}
ButtonStatus;