#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include "rsc_types.h"
#include "prugpio.h"

#define PRU0                            0x00
#define PRU1                            0x01
#define CORE_CLOCK                      (uint32_t)200000000
#define CLOCK_MS                        (uint32_t)CORE_CLOCK / 1000

#define USE_RPMSG_FRAMEWORK             1

#ifdef SELECT_PRU0
#warning "Current PRU: PRU0"
#define SHARED_DRAM_ADDR                0x10000
#define CURRENT_PRU                     PRU0
#define LED                             USR1
#else
#warning "Current PRU: PRU1"
#define SHARED_DRAM_ADDR                0x11800
#define CURRENT_PRU                     PRU1
#define LED                             USR3
#endif

#ifdef SELECT_PRU0
#define SBNI_TX_P                       (1<<1)
#define SBNI_TX_N                       (1<<0)
#define SBNI_RX_PIN                     2
#define HOST_INTERRUPT                  30
#else
#define SBNI_TX_P                       (1<<1)
#define SBNI_TX_N                       (1<<0)
#define SBNI_RX_PIN                     2
#define HOST_INTERRUPT                  31
#endif

#define SBNI_TX                     SBNI_TX_P + SBNI_TX_N
#define SBNI_RX                     (1 << SBNI_RX_PIN)

#define EXTERNAL_EVENT_RX_PENDING   1
#define EXTERNAL_EVENT_HOST_INTR    HOST_INTERRUPT

#endif
