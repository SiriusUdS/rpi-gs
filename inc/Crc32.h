#pragma once

#include <stdint.h>
#include <stddef.h>

class Crc32 {
public:
    /**
     * @brief Computes the MPEG-2 CRC-32 value of a data buffer.
     * @param data Pointer to the buffer.
     * @param length Number of bytes in the buffer.
     * @param initial_crc Initial CRC value (defaults to 0xFFFFFFFF).
     * @return The computed CRC-32 value.
     */
    static uint32_t calculate(const uint8_t* data, size_t length, uint32_t initial_crc = 0xFFFFFFFF);
};
