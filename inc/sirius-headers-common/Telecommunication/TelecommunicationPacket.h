#pragma once
/**
 * @deprecated DO NOT USE THIS FILE IN THE ETHERNET ARCHITECTURE
 * 2025 competition.
 */
#include "TelecommunicationStatus.h"
#include "TelecommunicationErrorStatus.h"

#include "TelemetryHeader.h"

typedef struct {
  TelecommunicationErrorStatus errorStatus;
  TelecommunicationStatus      status;

  uint32_t timestamp_ms;
}
TelecommunicationPacketData;

typedef struct {
  TelemetryHeader             header;
  TelecommunicationPacketData rawData;
  uint32_t                    crc;
}
TelecommunicationPacketFields;

typedef union {
  TelecommunicationPacketFields fields;

  uint32_t data32[sizeof(TelecommunicationPacketFields) / sizeof(uint32_t)];

  uint8_t data[sizeof(TelecommunicationPacketFields)];
}
TelecommunicationPacket;