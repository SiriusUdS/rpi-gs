#pragma once
#include <stdint.h>



typedef struct {
    uint8_t forceState:1;
    uint8_t confirmSync:1;
    uint8_t syncTime:1;
    uint8_t diagnose:1;
    uint8_t reserved:4;
} FrameSyncFlags;


typedef union {
    FrameSyncFlags frame;
    uint8_t code;
} SyncFlags;

typedef struct {
    uint8_t deviceIDFrom:8;
    uint8_t deviceIDTo:8;
    uint8_t syncState:8;
    SyncFlags syncFlags;
} FrameSyncPacket;

/// @brief This packet sync the desired device with the state from another. Every devices on the network MUST treat this packet.
typedef union {
    FrameSyncPacket frame;
    uint8_t data[sizeof(FrameSyncPacket)];
} SyncPacket;