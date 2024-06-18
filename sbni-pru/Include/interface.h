#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "if_stats.h"

#define SBNI_SPEED_125              DIRECT_IO_LATENCY_0    //  8 мкс
#define SBNI_SPEED_250              DIRECT_IO_LATENCY_1    //  4 мкс
#define SBNI_SPEED_500              DIRECT_IO_LATENCY_2    //  2 мкс
#define SBNI_SPEED_1000             DIRECT_IO_LATENCY_3    //  1 мкс
#define SBNI_SPEED_2000             DIRECT_IO_LATENCY_4    // 500 нс
#define SBNI_SPEED_3125             DIRECT_IO_LATENCY_5    // 320 нс
#define SBNI_SPEED_4166             DIRECT_IO_LATENCY_6    // 240 нс

#define TO_HOST_EVENT_RX_PENDING    0x01
#define TO_HOST_EVENT_TX_DONE       0x02
#define TO_HOST_EVENT_SPEED_SET     0x03

#define FROM_HOST_EVENT_NONE        0x00
#define FROM_HOST_EVENT_TX_REQUEST  0x01
#define FROM_HOST_EVENT_SPEED_SET   0x02

#define RPMSG_ARM_PORT              (unsigned short)1024

#ifdef SELECT_PRU0
#define RPMSG_PRU_PORT              (unsigned short)30
#endif
#ifdef SELECT_PRU1
#define RPMSG_PRU_PORT              (unsigned short)31
#endif

enum if_status
{
    IF_STATUS_FREE = 0,
    IF_STATUS_BUSY = 1
};

#define CHANGE_IF_STATUS(new_status)    \
{                                       \
    shared_dram->status = new_status;   \
}

typedef struct
{
    uint8_t   rx_buffer[BUFFER_SIZE];     // 2048
    uint8_t   tx_buffer[BUFFER_SIZE];     // 4096
    uint16_t  rx_bytes;                   // 4098
    uint16_t  tx_bytes;                   // 4100
    struct if_stats stats;                // 4140
    enum if_status status;                // 4141
    uint8_t   sbni_speed;                 // 4145
    uint8_t   mac_addr[6];                // 4152
    uint8_t   reserved[8];                // 4160
} HOST_MEM;

void pru_init_all(void);
void pru_interface_speed_set(uint8_t value);
void pru_interface_data_send(uint8_t *buffer, uint16_t buffer_size);
uint16_t pru_interface_data_read(uint8_t *buffer);
void pru_interface_stats_update(HOST_MEM *shared_dram, int8_t stats_entry);

#ifdef USE_RPMSG_FRAMEWORK
void arm_host_message_send(uint8_t *rpmsg_payload, uint16_t rpmsg_len);
#else
void arm_host_message_send(HOST_MEM *shared_dram, uint8_t event);
#endif
void arm_host_message_handle(HOST_MEM *host_message);

#endif
