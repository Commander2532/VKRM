#include "main.h"
#include "asmfunc.h"
#include "interface.h"
#include "sbni.h"

HOST_MEM *shared_dram;

void main(void)
{
    unsigned char external_event = 0;

    pru_init_all();

    shared_dram = (HOST_MEM *)SHARED_DRAM_ADDR;

    while(1)
    {
        external_event = __event_wait();

        //CHANGE_IF_STATUS(IF_STATUS_BUSY);

        switch(external_event)
        {
            case EXTERNAL_EVENT_RX_PENDING:
            {
                uint16_t bytes_received = sbni_data_receive(shared_dram->rx_buffer);
                if (bytes_received)
                {
                    shared_dram->rx_bytes = bytes_received;
                    uint8_t to_arm_event = TO_HOST_EVENT_RX_PENDING;
                    arm_host_message_send(&to_arm_event, 1);
                }
                break;
            }
            case EXTERNAL_EVENT_HOST_INTR:
            {
                CHANGE_IF_STATUS(IF_STATUS_BUSY);
                pru_conf_int_clear();
                arm_host_message_handle(shared_dram);
                break;
            }
        }

        //CHANGE_IF_STATUS(IF_STATUS_FREE);
    }
}
