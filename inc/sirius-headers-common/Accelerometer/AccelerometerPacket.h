#pragma once

#include "AccelerometerData.h"
#include "AccelerometerStatus.h"
#include "AccelerometerErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  AccelerometerData data;

  AccelerometerErrorStatus  errorStatus;
  AccelerometerStatus       status;

  uint32_t timeStamp_ms;
}
AccelerometerPacketData;

typedef struct {
  TelemetryHeader         header;
  AccelerometerPacketData rawData;
  uint32_t                crc;
}
AccelerometerPacketFields;

typedef union {
  AccelerometerPacketFields fields;

  uint8_t data[sizeof(AccelerometerPacketFields)];
}
AccelerometerPacket;