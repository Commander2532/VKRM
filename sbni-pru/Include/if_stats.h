#ifndef _IFSTATS_H_
#define _IFSTATS_H_

#include <stdint.h>

struct if_stats
{
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t sbni_len_errors;
    uint64_t sbni_marker_errors;
    uint64_t sbni_crc_errors;
};

#define SBNI_PACKETS_SENT            0
#define SBNI_PACKETS_RECEIVED        1
#define SBNI_LENGTH_ERRORS          -1
#define SBNI_MARKER_ERRORS          -2
#define SBNI_CRC_ERRORS             -3

#endif
