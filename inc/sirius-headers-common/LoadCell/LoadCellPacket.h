#pragma once

#include "LoadCellData.h"
#include "LoadCellStatus.h"
#include "LoadCellErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  LoadCellData data;

  LoadCellErrorStatus errorStatus;
  LoadCellStatus      status;

  uint32_t timeStamp_ms;
}
LoadCellPacketData;

typedef struct {
  TelemetryHeader    header;
  LoadCellPacketData rawData;
  uint32_t           crc;
}
LoadCellPacketFields;

typedef union {
  LoadCellPacketFields fields;

  uint8_t data[sizeof(LoadCellPacketFields)];
}
LoadCellPacket;