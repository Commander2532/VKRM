#ifndef _THREADING_H_
#define _THREADING_H_

#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>

#define __NOP {}

int sbni_threads_create(uint8_t);
void sbni_threads_destroy(uint8_t);

void *sbni_rx_handler(uint8_t);
void *sbni_tx_handler(uint8_t);

#endif