#pragma once
/***
 * @deprecated DO NOT USE THIS FILE IN THE ETHERNET ARCHITECTURE
 * 2025 competition file
 */
#include <stdint.h>

#define GS_COMMAND_TYPE_CODE 0x47834

typedef union {
  struct {
    uint32_t type         : 20;
    uint32_t commandIndex : 4;
    uint32_t boardId      : 3;
    uint32_t commandCode  : 5;
  }
  bits;

  uint32_t value;
}
GSCommandHeader;

typedef struct {
  GSCommandHeader header;
  uint8_t         padding[36];
  uint32_t        crc;
}
GSCommandFields;

typedef union {
  GSCommandFields fields;

  uint32_t data32[sizeof(GSCommandFields) / sizeof(uint32_t)];

  uint8_t data[sizeof(GSCommandFields)];
}
GSCommand;