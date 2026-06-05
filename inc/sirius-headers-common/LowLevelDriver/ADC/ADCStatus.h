#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t dmaHalfFull : 1;
    uint16_t dmaFull : 1;
    uint16_t RESERVED : 14;
  }
  bits;
  uint16_t value;
}
ADCStatus;