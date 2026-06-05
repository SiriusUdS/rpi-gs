#pragma once

#include "GpsData.h"
#include "GPSStatus.h"
#include "GPSErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  GpsData data;

  GPSErrorStatus errorStatus;
  GpsStatus      status;

  uint32_t timeStamp_ms;
}
GpsPacketData;

typedef struct {
  TelemetryHeader header;
  GpsPacketData   rawData;
  uint32_t        crc;
}
GpsPacketFields;

typedef union {
  GpsPacketFields fields;

  uint8_t data[sizeof(GpsPacketFields)];
}
GpsPacket;