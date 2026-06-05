#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t RESERVED : 2;

    uint16_t state : 6;

    uint16_t isValveStartButtonPressed: 1;
    uint16_t isAllowFillSwitchOn : 1;
    uint16_t isArmServoSwitchOn : 1;
    uint16_t isArmIgniterSwitchOn : 1;

    uint16_t isAllowDumpSwitchOn : 1;
    uint16_t isEmergencyStopButtonPressed : 1;
    uint16_t isFireIgniterButtonPressed : 1;
    uint16_t isUnsafeKeySwitchPressed : 1;
  }
  bits;

  uint16_t value;
}
GSControlStatus;