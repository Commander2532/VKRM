#include "threading.h"
#include "prumsg.h"
#include "errors.h"
#include "io.h"

pthread_t sbni_rx_thread[2];
pthread_t sbni_tx_thread[2];

int sbni_threads_create(uint8_t num)
{
    int ret;
    
    ret = pthread_create(&(sbni_rx_thread[num]), NULL, (void *)sbni_rx_handler, (void *)&num);
    
    if (ret)
        return ret;

    ret = pthread_create(&(sbni_tx_thread[num]), NULL, (void *)sbni_tx_handler, (void *)&num);
    
    return ret;
}

void sbni_threads_destroy(uint8_t num)
{
    pthread_cancel(sbni_rx_thread[num]);
    pthread_cancel(sbni_tx_thread[num]);
}

void *sbni_rx_handler(uint8_t num)
{
    uint8_t rx_buffer[MAX_BUFFER_SIZE];

    while (1)
    {
        uint8_t event = pru_msg_event_get(num);
        switch(event)
        {
            case FROM_PRU_EVENT_RX_PENDING:
            {
                uint16_t rx_bytes = pru_rx_data_get(num, rx_buffer);
                if (rx_bytes > 0 && rx_bytes < MAX_BUFFER_SIZE)
                    sbni_io_write(num, rx_buffer, rx_bytes);
				sbni_errors[0].sbni_packets_received++;
                break;
            }
            case FROM_PRU_EVENT_TX_DONE:
            {
				sbni_errors[0].sbni_packets_sent++;
                break;
            }
            case FROM_PRU_EVENT_SPEED_SET:
            {
                printf("sbni pruss: sbni%d speed set to %d", num, pru_speed_get(num));
                break;
            }
        }
    }
}

void *sbni_tx_handler(uint8_t num)
{
    uint8_t tx_buffer[MAX_BUFFER_SIZE];

    while(1)
    {
        size_t tx_bytes = sbni_io_read(num, tx_buffer);

        if (tx_bytes > 0)
            pru_tx_data_set(num, tx_buffer, tx_bytes);
    }
}
