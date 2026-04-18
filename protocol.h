#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

/* Packet format: [Header:0xAA][Type:1Byte][Len:1Byte][Payload:UpTo255][CRC8] */
#define PROTOCOL_HEADER     0xAA
#define PROTOCOL_MAX_PAYLOAD  255
#define PROTOCOL_MIN_PACKET_SIZE 4  /* Header + Type + Len + CRC */

/* Message types */
typedef enum {
    PROTOCOL_TYPE_SENSOR_DATA = 0x01,
    PROTOCOL_TYPE_SYSTEM_EVENT = 0x02,
    PROTOCOL_TYPE_ERROR = 0x03
} protocol_type_t;

/* Packet structure (for reference, not used for transmission due to packing) */
typedef struct {
    uint8_t header;
    uint8_t type;
    uint8_t length;
    uint8_t payload[PROTOCOL_MAX_PAYLOAD];
    uint8_t crc;
} protocol_packet_t;

/**
 * @brief Calculate CRC-8 over data buffer
 * @param data Pointer to data buffer
 * @param len Length of data buffer
 * @return Calculated CRC-8 value
 */
uint8_t crc8_calc(const uint8_t* data, uint8_t len);

/**
 * @brief Encode a uint32_t sensor value into a binary packet
 * @param buffer Output buffer (must be at least PROTOCOL_MIN_PACKET_SIZE + 4 bytes)
 * @param type Protocol message type
 * @param sensor_value The 32-bit sensor value to encode
 * @return Total packet length including header and CRC
 */
uint8_t protocol_encode(uint8_t* buffer, protocol_type_t type, uint32_t sensor_value);

#endif /* PROTOCOL_H */
