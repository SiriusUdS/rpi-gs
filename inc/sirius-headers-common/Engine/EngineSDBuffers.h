#pragma once

#include <stdint.h>

#define SD_CARD_BUFFER_SIZE_BYTES 0x10000
#define SD_CARD_FOOTER_SIZE_BYTES 0x80
#define SD_CARD_FOOTER_SIGNATURE_SIZE_BYTES 0x40

typedef struct {
  uint32_t timestamp_ms;
  uint16_t status;
  uint16_t errorStatus;
  uint16_t valveStatus[2];
  uint16_t valveErrorStatus[2];
  uint32_t currentCommand[3];
  uint8_t padding[32];
  uint32_t signature[SD_CARD_FOOTER_SIGNATURE_SIZE_BYTES / (sizeof(uint32_t))];
  uint32_t crc;
}
EngineSDBufferFooter;

typedef struct {
  uint8_t data[(SD_CARD_BUFFER_SIZE_BYTES / 2) - sizeof(EngineSDBufferFooter)];
  EngineSDBufferFooter footer;
}
EngineSDFormattedData;

typedef union {
  uint16_t values[SD_CARD_BUFFER_SIZE_BYTES / sizeof(uint16_t)];

  uint32_t values32[SD_CARD_BUFFER_SIZE_BYTES / sizeof(uint32_t)];

  EngineSDFormattedData sdData[2];

  uint8_t hex[SD_CARD_BUFFER_SIZE_BYTES];
}
EngineSDCardBuffer;