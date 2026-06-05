#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t notInitialized : 1;
    uint16_t invalidFunctionPointer : 1;
    uint16_t notConnected : 1;
    uint16_t RESERVED : 13;
  }
  bits;
  uint16_t value;
}
AccelerometerStatus;