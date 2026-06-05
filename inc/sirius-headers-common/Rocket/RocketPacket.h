#pragma once

#include "RocketStatus.h"
#include "RocketErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

#define STORAGE_DEVICE_AMOUNT 2

typedef struct {
  RocketErrorStatus  errorStatus;
  RocketStatus       status;

  uint32_t           timeStamp_ms;
}
RocketPacketData;

typedef union {
  TelemetryHeader header;
  RocketPacketData rawData;
  uint32_t crc;
}
RocketPacketFields;

typedef union {
  RocketPacketFields fields;

  uint8_t data[sizeof(RocketPacketFields)];
}
RocketPacket;