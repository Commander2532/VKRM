#include "errors.h"

struct sbni_errors sbni_errors[2];

void errors_print(void)
{
    printf("\n\r");
    printf("SBNI0:\n\r");
    printf("SBNI_PACKETS_SENT     : %i\r\n", sbni_errors[0].sbni_packets_sent);
    printf("SBNI_PACKETS_RECEIVED : %i\r\n", sbni_errors[0].sbni_packets_received);
    printf("SBNI_LENGTH_ERRORS    : %i\n\r", sbni_errors[0].sbni_length_errors);
    printf("SBNI_MARKER_ERRORS    : %i\n\r", sbni_errors[0].sbni_marker_errors);
    printf("SBNI_CRC_ERRORS       : %i\n\r", sbni_errors[0].sbni_crc_errors);
    printf("\n\r");
    printf("SBNI1:\n\r");
    printf("SBNI_PACKETS_SENT     : %i\r\n", sbni_errors[1].sbni_packets_sent);
    printf("SBNI_PACKETS_RECEIVED : %i\r\n", sbni_errors[1].sbni_packets_received);
    printf("SBNI_LENGTH_ERRORS    : %i\n\r", sbni_errors[1].sbni_length_errors);
    printf("SBNI_MARKER_ERRORS    : %i\n\r", sbni_errors[1].sbni_marker_errors);
    printf("SBNI_CRC_ERRORS       : %i\n\r", sbni_errors[1].sbni_crc_errors);
    printf("\n\r");
}
