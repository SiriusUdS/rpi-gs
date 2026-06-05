#pragma once

#include "ButtonStatus.h"
#include "ButtonErrorStatus.h"

#include "../Telecommunication/TelemetryHeader.h"

typedef struct {
  ButtonErrorStatus errorStatus;
  ButtonStatus status;

  uint32_t timeStamp_ms;
}
ButtonPacketData;

typedef struct {
  TelemetryHeader  header;
  ButtonPacketData rawData;
  uint32_t         crc;
}
ButtonPacketFields;

typedef union {
  ButtonPacketFields fields;

  uint8_t data[sizeof(ButtonPacketFields)];
}
ButtonPacket;