#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <errno.h>
#include <string.h>
#include <stdio.h>

struct sbni_errors
{
    unsigned int sbni_packets_sent;
    unsigned int sbni_packets_received;
    unsigned int sbni_length_errors;
    unsigned int sbni_marker_errors;
    unsigned int sbni_crc_errors;
};

//#define SBNI_PACKETS_SENT            0
//#define SBNI_PACKETS_RECEIVED        1
//#define SBNI_LENGTH_ERRORS          -20
//#define SBNI_MARKER_ERRORS          -21
//#define SBNI_CRC_ERRORS             -22

extern struct sbni_errors sbni_errors[2];

#endif