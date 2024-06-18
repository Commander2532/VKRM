#ifndef _SBNI_PHYS_H_
#define _SBNI_PHYS_H_

#include "prugpio.h"
#include "pru_cfg.h"
#include "pru_iep.h"
#include "pru_intc.h"
#include <pru_rpmsg.h>
#include <stdbool.h>
#include <stdint.h>

#define DIRECT_IO_LATENCY_0         0x00
#define DIRECT_IO_LATENCY_1         0x01
#define DIRECT_IO_LATENCY_2         0x02
#define DIRECT_IO_LATENCY_3         0x03
#define DIRECT_IO_LATENCY_4         0x04
#define DIRECT_IO_LATENCY_5         0x05
#define DIRECT_IO_LATENCY_6         0x06
#define DIRECT_IO_LATENCY_7         0x07

#define BUFFER_SIZE                 2048

#define PRU_ARM_INTERRUPT
#define INT_HOST_NUM                2
#define INT_CHAN_NUM                2

#define __NOP                       {}

#ifdef USE_RPMSG_FRAMEWORK
void pru_conf_sys_init(struct pru_rpmsg_transport *transport);
#else
void pru_conf_sys_init(void);
#endif
void pru_conf_reset_pins(void);
void pru_conf_set_latency(unsigned char value);
void pru_conf_int_clear(void);

#endif
