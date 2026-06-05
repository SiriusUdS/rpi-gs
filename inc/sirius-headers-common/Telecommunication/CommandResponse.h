#pragma once
/**
 * @deprecated DO NOT USE THIS FILE IN THE ETHERNET ARCHITECTURE
 * 2025 file competition
 */
#include <stdint.h>

#define COMMAND_RESPONSE_TYPE_CODE 0x52535

#define RESPONSE_CODE_OK 0x00

typedef union {
  struct {
    uint32_t type         : 20;
    uint32_t commandIndex : 4;
    uint32_t boardId      : 3;
    uint32_t response     : 5;
  }
  bits;

  uint32_t value;
}
CommandResponseHeader;

typedef struct {
  CommandResponseHeader header;
  uint8_t               padding[36];
  uint32_t              crc;
}
CommandResponseFields;

typedef union {
  CommandResponseFields fields;

  uint32_t data32[sizeof(CommandResponseFields) / sizeof(uint32_t)];

  uint8_t data[sizeof(CommandResponseFields)];
}
CommandResponse;