#pragma once

#include "MagnetometerData.h"
#include "MagnetometerStatus.h"
#include "MagnetometerErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  MagnetometerData data;

  MagnetometerErrorStatus errorStatus;
  MagnetometerStatus      status;

  uint32_t timeStamp_ms;
}
MagnetometerPacketData;

typedef struct {
  TelemetryHeader        header;
  MagnetometerPacketData rawData;
  uint32_t               crc;
}
MagnetometerPacketFields;

typedef union {
  MagnetometerPacketFields fields;

  uint8_t data[sizeof(MagnetometerPacketFields)];
}
MagnetometerPacket;