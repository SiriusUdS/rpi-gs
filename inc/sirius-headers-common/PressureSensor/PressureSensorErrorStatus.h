#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t notInitialized         : 1;
    uint16_t nullFunctionPointer    : 1;
    uint16_t defaultFunctionCalled  : 1;
    uint16_t notConnected           : 1;

    uint16_t leakSuspected    : 1;
    uint16_t leakDetected     : 1;
    uint16_t belowMinPressure : 1;
    uint16_t aboveMaxPressure : 1;

    uint16_t RESERVED : 8;
  }
  bits;

  uint16_t value;
}
PressureSensorErrorStatus;