#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t notInitialized : 1;
    uint16_t invalidState : 1;
    uint16_t testFailed : 1;
    uint16_t RESERVED : 13;
  }
  bits;
  
  uint16_t value;
}
EngineErrorStatus;