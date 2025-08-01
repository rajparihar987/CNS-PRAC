#ifndef CRC_H
#define CRC_H

#include <string>
#include <cstdint>

#define CRC_POLY 0x07  // CRC-8 polynomial

uint8_t computeCRC(const std::string& data) {
    uint8_t crc = 0x00;
    for (char ch : data) {
        crc ^= static_cast<uint8_t>(ch);
        for (int i = 0; i < 8; i++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC_POLY;
            else
                crc <<= 1;
        }
    }
    return crc;
}

#endif
