#pragma once

#include "GyroscopeData.h"
#include "GyroscopeStatus.h"
#include "GyroscopeErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  GyroscopeAxis data;

  GyroscopeErrorStatus errorStatus;
  GyroscopeStatus      status;

  uint32_t timeStamp_ms;
}
GyroscopeData;

typedef struct {
  TelemetryHeader header;
  GyroscopeData   rawData;
  uint32_t        crc;
}
GyroscopePacketFields;

typedef union {
  GyroscopePacketFields fields;

  uint8_t data[sizeof(GyroscopePacketFields)];
}
GyroscopePacket;