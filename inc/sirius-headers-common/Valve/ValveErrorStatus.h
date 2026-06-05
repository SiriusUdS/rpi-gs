#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t notInitialized        : 1;
    uint16_t nullFunctionPointer   : 1;
    uint16_t defaultFunctionCalled : 1;
    uint16_t notConnected          : 1;

    uint16_t dutyCycleBelowMin      : 1;
    uint16_t dutyCycleOverMax       : 1;
    uint16_t exceededMaxAdjustments : 1;
    uint16_t invalidState           : 1;

    uint16_t RESERVED : 8;
  }
  bits;

  uint16_t value;
}
ValveErrorStatus;