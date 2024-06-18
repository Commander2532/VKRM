#include "pru_sbni.h"
#include "sbni_gpio.h"

pruCfg PRU_CFG;
unsigned int SBNI_SPEED;
unsigned int BYTE_LENGTH;
unsigned int HALF_BYTE_LENGTH;
unsigned char TX_P = TX_P_NUM;
unsigned char TX_N = TX_N_NUM;

extern HOSTMSG *hostMessage;

unsigned char current_speed = 1;
unsigned char cpu_num = (unsigned char)CURRENT_PRU;

unsigned char tx_buffer[BUFFER_SIZE];
int tx_pointer = 1;

unsigned char rx_buffer[BUFFER_SIZE];
int rx_pointer = BUFFER_SIZE-1;

void pru_sbni_send_byte(unsigned char byte)
{
    __transmit_byte(byte);
}

void pru_sbni_send_data(unsigned char* buffer, unsigned int buffer_size)
{
    memcpy(tx_buffer, buffer, buffer_size);
    hostMessage->tx_pending = 0;
    tx_pointer = buffer_size-1;
    while (tx_pointer >= 0)
        pru_sbni_send_byte(tx_buffer[tx_pointer--]);
    hostMessage->tx_done = 1;
}

unsigned char pru_sbni_read_byte(void)
{
    return __receive_byte();
}

void pru_sbni_read_data(void)
{
    while (rx_pointer >= 0)
    {
        unsigned char data = pru_sbni_read_byte();
        rx_buffer[rx_pointer--] = data;
    }
    memcpy(hostMessage->data_buffer, rx_buffer, BUFFER_SIZE);
    hostMessage->rx_done = 1;
    hostMessage->rx_pending = 0;
}

void pru_sbni_speed_apply(unsigned char value)
{
    switch(value)
    {
        case SBNI_SPEED_125:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x1E;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x1E;
            SBNI_SPEED = 125000;
            break;
        }
        case SBNI_SPEED_250:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x0E;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x0E;
            SBNI_SPEED = 250000;
            break;
        }
        case SBNI_SPEED_500:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x06;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x06;
            SBNI_SPEED = 500000;
            break;
        }
        case SBNI_SPEED_1000:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x02;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x02;
            SBNI_SPEED = 1000000;
            break;
        }
        case SBNI_SPEED_2000:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x00;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x17;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x00;
            SBNI_SPEED = 2000000;
            break;
        }
        case SBNI_SPEED_3125:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x0E;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x00;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x0E;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x00;
            break;
        }
        case SBNI_SPEED_4166:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x0A;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x00;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x0A;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x00;
            SBNI_SPEED = 4166666;
            break;
        }
        case SBNI_SPEED_5000:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x08;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x00;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x08;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x00;
            SBNI_SPEED = 5000000;
            break;
        }
        case SBNI_SPEED_6250:
        {
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV0 = 0x06;
            PRU_CFG.GPCFG0_bit.PRU0_GPI_DIV1 = 0x00;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV0 = 0x06;
            PRU_CFG.GPCFG1_bit.PRU1_GPI_DIV1 = 0x00;
            SBNI_SPEED = 6250000;
            break;
        }
    }
    BYTE_LENGTH = CORE_CLOCK / SBNI_SPEED;
    HALF_BYTE_LENGTH = BYTE_LENGTH / 2;
}

void pru_sbni_init(void)
{
    /* Инициализация прерываний */
    CT_INTC.SIPR0 = 0xFFFFFFFF;
    CT_INTC.CMR1 = 0x00000000;
    CT_INTC.CMR1_bit.CH_MAP_7 = 0x00;
    CT_INTC.HMR0_bit.HINT_MAP_0 = 0x00;
    CT_INTC.ESR0 = 0x80;
    CT_INTC.HIER = 0x01;
    CT_INTC.GER = 0x01;
    CT_INTC.SECR0 = 0xFFFFFFFF;
    CT_INTC.SECR1 = 0xFFFFFFFF;

    /* Инициализация таймера */
/*  CT_IEP.TMR_GLB_CFG_bit.CNT_EN = 0x00;
    CT_IEP.TMR_CNT = 0xFFFFFFFF;
    CT_IEP.TMR_GLB_STS_bit.CNT_OVF = 0x01;
    CT_IEP.TMR_CMP_STS_bit.CMP_HIT = 0xFF;
    CT_IEP.TMR_COMPEN_bit.COMPEN_CNT = 0x00;
    CT_IEP.TMR_CMP_CFG_bit.CMP0_RST_CNT_EN = 0x01;
    CT_IEP.TMR_CMP_CFG_bit.CMP_EN = 0x01;
    CT_IEP.TMR_CMP0 = 0xC7;
    CT_IEP.TMR_GLB_CFG = 0x11; */

    PRU_CFG.SYSCFG_bit.STANDBY_INIT = 0;
    PRU_CFG.SYSCFG_bit.STANDBY_MODE = 1;
    PRU_CFG.SYSCFG_bit.IDLE_MODE = 1;

    PRU_CFG.GPCFG0_bit.PRU0_GPO_MODE = 0x01;
    PRU_CFG.GPCFG1_bit.PRU1_GPO_MODE = 0x01;
    PRU_CFG.GPCFG0_bit.PRU0_GPI_MODE = 0x02;
    PRU_CFG.GPCFG1_bit.PRU1_GPI_MODE = 0x02;

    pru_sbni_speed_apply(current_speed);
}

void pru_sbni_speed_set(unsigned char speed)
{
    if (current_speed!=speed)
    {
        current_speed=speed;
        pru_sbni_speed_apply(current_speed);
    }
}

unsigned char pru_sbni_speed_get(void)
{
    return current_speed;
}
