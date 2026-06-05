#pragma once

#include "AltimeterData.h"
#include "AltimeterStatus.h"
#include "AltimeterErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  AltimeterData         data;

  AltimeterErrorStatus  errorStatus;
  AltimeterStatus       status;

  uint32_t timeStamp_ms;
}
AltimeterPacketData;

typedef struct {
  TelemetryHeader     header;
  AltimeterPacketData rawData;
  uint32_t            crc;
}
AltimeterPacketFields;

typedef union {
  AltimeterPacketFields fields;

  uint8_t data[sizeof(AltimeterPacketFields)];
}
AltimeterPacket;