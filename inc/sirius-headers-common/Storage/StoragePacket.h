#pragma once

#include "StorageStatus.h"
#include "StorageErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  StorageErrorStatus  errorStatus;
  StorageStatus       status;

  uint32_t timeStamp_ms;
}
StoragePacketData;

typedef struct {
  TelemetryHeader   header;
  StoragePacketData rawData;
  uint32_t          crc;
}
StoragePacketFields;

typedef union {
  StoragePacketFields fields;

  uint8_t data[sizeof(StoragePacketFields)];
}
StoragePacket;