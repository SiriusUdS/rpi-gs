#pragma once

#include <stdint.h>

typedef union {
  struct {
    uint16_t notInitialized : 1;
    uint16_t nullFunctionPointer : 1;
    uint16_t defaultFunctionCalled : 1;
    uint16_t disconnected : 1;

    uint16_t isFull : 1;
    uint16_t writeFailed : 1;
    uint16_t readFailed : 1;
    uint16_t incompleteWrite : 1;

    uint16_t fs_mountFailed : 1;
    uint16_t fs_openFailed : 1;
    uint16_t fs_syncFailed : 1;
    uint16_t fs_closeFailed : 1;

    uint16_t fs_createDirectoryFail : 1;
    uint16_t fs_fileTooLarge : 1;
    uint16_t fs_unexpectedFileName : 1;
    uint16_t RESERVED : 1;
  }
  bits;
  
  uint16_t value;
}
StorageErrorStatus;