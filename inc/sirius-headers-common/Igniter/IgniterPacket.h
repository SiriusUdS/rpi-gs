#pragma once

#include "IgniterStatus.h"
#include "IgniterErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  IgniterErrorStatus errorStatus;
  IgniterStatus      status;

  uint32_t timeStamp_ms;
}
IgniterPacketData;

typedef struct {
  TelemetryHeader   header;
  IgniterPacketData rawData;
  uint32_t          crc;
}
IgniterPacketFields;

typedef union {
  IgniterPacketFields fields;

  uint8_t data[sizeof(IgniterPacketFields)];
}
IgniterPacket;