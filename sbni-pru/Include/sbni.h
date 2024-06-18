#ifndef _SBNI_H_
#define _SBNI_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define SBNI_MARKER                     0x5A
#define SBNI_MAX_DATA_SIZE              1012

#define SBNI_FRAME_VALID                0
#define SBNI_FRAME_SERVICE              1

#define SBNI_TX_OK                      0
#define SBNI_TX_ERR                    -1

typedef struct
{
    uint16_t  length       : 10;
    uint16_t  unused       : 1;
    uint16_t  repeat       : 1;
    uint16_t  request      : 3;
    uint16_t  first        : 1;

    uint8_t   number;
    uint8_t   reserved;
} __attribute__((packed)) SBNI_FRAME_SUBHEADER;

typedef struct
{
    uint8_t   marker;
    union
    {
        SBNI_FRAME_SUBHEADER subheader;
        uint8_t raw_subheader[sizeof(SBNI_FRAME_SUBHEADER)];
    };
} __attribute__((packed)) SBNI_FRAME_HEADER;

typedef struct
{
    SBNI_FRAME_HEADER *header;
    uint8_t           *data;
    uint32_t          *crc;
} __attribute__((packed)) SBNI_FRAME;

#define SBNI_HLEN                       sizeof(SBNI_FRAME_HEADER)
#define SBNI_CRC_LEN                    sizeof(uint32_t)
#define SBNI_MIN_FRAME_SIZE             SBNI_HLEN + SBNI_CRC_LEN
#define SBNI_MAX_FRAME_SIZE             SBNI_MIN_FRAME_SIZE + SBNI_MAX_DATA_SIZE

void sbni_data_send(uint8_t *source_buffer, uint16_t data_size);
uint16_t sbni_data_receive(uint8_t *dest_buffer);

#endif
