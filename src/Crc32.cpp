#include "Crc32.h"
#include "system/crc32_polynomial.hpp"

uint32_t Crc32::calculate(const uint8_t* data, size_t length, uint32_t initial_crc) {
    std::uint32_t crc = initial_crc;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 1u) ? ((crc >> 1) ^ logic::data_integrity::CRC32_POLYNOMIAL_REFLECTED) : (crc >> 1);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}
