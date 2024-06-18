#ifndef _PRUMSG_H_
#define _PRUMSG_H_

#include <sys/file.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define SHARED_DRAM_ADDR                0x4A310000
#define SHARED_DRAM_SIZE                0x1800
#define MAP_SIZE			SHARED_DRAM_SIZE * 2
#define MAX_BUFFER_SIZE                 1012

#define RPROC1_FW_NAME                  "pru0-sbni-firmware.out"
#define RPROC2_FW_NAME                  "pru1-sbni-firmware.out"
#define RPROC1_FW_SET                   "/sys/class/remoteproc/remoteproc1/firmware"
#define RPROC2_FW_SET                   "/sys/class/remoteproc/remoteproc2/firmware"
#define RPROC1_CTL_FILE                 "/sys/class/remoteproc/remoteproc1/state"
#define RPROC2_CTL_FILE                 "/sys/class/remoteproc/remoteproc2/state"
#define SBNICTL_FILE			"/proc/sbni/ctl"
#define SBNI0_TX			"/proc/sbni/sbni0_rx"
#define SBNI0_RX			"/proc/sbni/sbni0_tx"
#define SBNI0_TX			"/proc/sbni/sbni1_rx"
#define SBNI0_RX			"/proc/sbni/sbni1_tx"

#define DEVICE_NAME_PRU0                "/dev/rpmsg_pru30"
#define DEVICE_NAME_PRU1                "/dev/rpmsg_pru31"
#define DEVICE_NAME_MEMORY              "/dev/mem

#define PRU_0                           0x00
#define PRU_1                           0x01

#define TO_PRU_EVENT_TX_REQUEST         0x01
#define TO_PRU_EVENT_SET_SPEED          0x02

#define FROM_PRU_EVENT_RX_PENDING       0x01
#define FROM_PRU_EVENT_TX_DONE          0x02
#define FROM_PRU_EVENT_SPEED_SET        0x03

struct ifstat
{
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t sbni_len_errors;
    uint64_t sbni_marker_errors;
    uint64_t sbni_crc_errors;
};

typedef struct
{
    uint8_t   rx_buffer[MAX_BUFFER_SIZE];       // 2048
    uint8_t   tx_buffer[MAX_BUFFER_SIZE];       // 4096
    uint16_t  bytes_received;                   // 4098
    uint16_t  bytes_to_send;                    // 4100
    struct ifstat stats;                        // 4140
    uint8_t   sbni_speed;                       // 4141
    uint8_t   reserved[19];                     // 4160
} PRU_SHARED_MEMORY;

typedef struct
{
    uint8_t speed_pru0;
    uint8_t speed_pru1;
    bool cmd_stop;
} sbnictl_mmap;

extern sbnictl_mmap *sbnictl;

int pru_memmap_init(void);
void pru_memmap_deinit(void);

int pru_rproc_init(uint8_t);
void pru_rproc_deinit(uint8_t);

void pru_speed_set(uint8_t, uint8_t);
int pru_speed_get(uint8_t);

void pru_msg_event_set(uint8_t, uint8_t);
uint8_t pru_msg_event_get(uint8_t);

void pru_tx_data_set(uint8_t, uint8_t*, uint16_t);
uint16_t pru_rx_data_get(uint8_t, uint8_t*);

#endif