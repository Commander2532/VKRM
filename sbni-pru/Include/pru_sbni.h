#ifndef _PRU_SBNI_H_
#define _PRU_SBNI_H_

#include <stdint.h>
#include "pru_intc.h"
#include "pru_cfg.h"
#include "pru_iep.h"

#define PRU1

#define RX_SB_DETECT                (1<<29)
#define EXT_INT_DETECT              (1<<30)
#define CORE_CLOCK                  200000000
#define BUFFER_SIZE                 1024
#define SBNI_SPEED_125              0x00
#define SBNI_SPEED_250              0x01
#define SBNI_SPEED_500              0x02
#define SBNI_SPEED_1000             0x03
#define SBNI_SPEED_2000             0x04
#define SBNI_SPEED_3125             0x05
#define SBNI_SPEED_4166             0x06
#define SBNI_SPEED_5000             0x07
#define SBNI_SPEED_6250             0x08

extern void __transmit_byte(unsigned char byte);
extern unsigned char __receive_byte(void);

typedef struct
{
    unsigned char rx_pending   : 1;
    unsigned char rx_done      : 1;
    unsigned char tx_pending   : 1;
    unsigned char tx_done      : 1;
    unsigned char upd_settings : 1;
    unsigned char reserved     : 3;
    unsigned char sbni_speed;
    unsigned char data_buffer[BUFFER_SIZE];
} HOSTMSG;

void pru_sbni_send_data(unsigned char* buffer, unsigned int buffer_size);
void pru_sbni_read_data(void);
void pru_sbni_speed_set(unsigned char speed);
void pru_sbni_init();

#endif
