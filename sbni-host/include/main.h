#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

#define SBNI_0              0
#define SBNI_1              1

int sbni_interface_open(uint8_t num);
void sbni_interface_close(uint8_t num);
void sbni_speed_set(int num, int speed);
int sbni_speed_get(int num);

#endif