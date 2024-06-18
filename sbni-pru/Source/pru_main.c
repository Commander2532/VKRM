#include "pru_main.h"

volatile register unsigned int __R30;
volatile register unsigned int __R31;

HOSTMSG *hostMessage;

int main(void)
{
    pru_sbni_init();
    __R30 |= SBNI_TX_P;
    __R30 &= ~SBNI_TX_N;

    while(1)
    {
        if (__R31 & RX_SB_DETECT != 0)
        {
            hostMessage->rx_pending = 1;
            pru_sbni_read_data();
        }
        else if (hostMessage->tx_pending > 0)
        {
            if (__R31 & RX_SB_DETECT == 0)
            {
                pru_sbni_send_data(hostMessage->data_buffer, BUFFER_SIZE);
            }
        }
        else if (hostMessage->upd_settings > 0)
        {
            pru_sbni_speed_set(hostMessage->sbni_speed);
            hostMessage->upd_settings = 0;
        }
    }
}
