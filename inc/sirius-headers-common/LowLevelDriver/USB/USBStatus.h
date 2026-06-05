#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t rxDataReady : 1;
    uint16_t RESERVED : 15;
  }
  bits;

  uint16_t value;
}
USBStatus;