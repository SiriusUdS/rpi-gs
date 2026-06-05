#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t testModeOn : 1;
    uint16_t fastModeOn : 1;
    uint16_t timestampBufferReady : 1;
    uint16_t slowModeBufferReady : 1;

    uint16_t state : 6;
    uint16_t RESERVED2 : 6;
  }
  bits;
  uint16_t value;
}
EngineStatus;