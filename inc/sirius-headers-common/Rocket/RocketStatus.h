#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t RESERVED : 16;
  }
  bits;
  
  uint16_t value;
}
RocketStatus;