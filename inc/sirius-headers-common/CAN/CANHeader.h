#pragma once
#include <stdint.h>

typedef struct {
    uint32_t deviceID:8;
    uint32_t deviceState:4;
    uint32_t messageID:8;
    uint32_t errorCtrl:2;
    uint32_t errorCode:7;
    uint32_t reserved:3;
}FrameCANHeader;

typedef union {
    FrameCANHeader frame;
    uint32_t code;
} CANHeader;