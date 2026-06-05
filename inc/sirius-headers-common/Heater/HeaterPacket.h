#pragma once

#include "HeaterStatus.h"
#include "HeaterErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  HeaterErrorStatus errorStatus;
  HeaterStatus      status;

  uint32_t timeStamp_ms;
}
HeaterPacketData;

typedef struct {
  TelemetryHeader  header;
  HeaterPacketData rawData;
  uint32_t         crc;
}
HeaterPacketFields;

typedef union {
  HeaterPacketFields fields;

  uint8_t data[sizeof(HeaterPacketFields)];
}
HeaterPacket;