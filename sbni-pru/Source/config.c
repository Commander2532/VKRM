#include "main.h"
#include "config.h"
#include "resource_table.h"

const uint8_t pru_num = CURRENT_PRU;
uint32_t tx_half_delay, tx_full_delay, rx_delay, rx_timeout;

volatile register uint32_t __R30;
volatile register uint32_t __R31;

volatile far pruIntc CT_INTC __attribute__((cregister("PRU_INTC", far), peripheral));
volatile pruCfg CT_CFG __attribute__((cregister("PRU_CFG", near), peripheral));

/*
 * @ brief: Начальная инициализация PRU
 * @ param: Нет
 * @ ret  : Нет
 */
void pru_conf_sys_init(struct pru_rpmsg_transport *transport)
{
    /* Инициализация настроечных регистров */
    CT_CFG.GPCFG0 = 0;
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
    CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

    /* Создание канала RPMSG для связи с хостом */
    uint8_t *status;
    status = &resourceTable.rpmsg_vdev.status;
    while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));
    pru_rpmsg_init(transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);
    while(pru_rpmsg_channel(RPMSG_NS_CREATE, transport, CHAN_NAME, CHAN_PORT) != PRU_RPMSG_SUCCESS);
}

/*
 * @ brief: Сброс уровня напряжения на выводах по умолчанию
 * @ param: Нет
 * @ ret  : Нет
 */
void pru_conf_reset_pins(void)
{
    __R30 &= !SBNI_TX_P;
    __R30 |= SBNI_TX_N;
}

/*
 * @ brief: Настройка задержек для ввода/вывода в зависимости
 *          от выбранной скорости
 * @ param: value - значение выбранной задержки
 * @ ret  : Нет
 */
void pru_conf_set_latency(unsigned char value)
{
    switch(value)
    {
        case DIRECT_IO_LATENCY_0:
        {
            tx_half_delay = 396;
            tx_full_delay = 792;
            rx_delay = 594;
            rx_timeout = 792;
            break;
        }
        case DIRECT_IO_LATENCY_1:
        {
            tx_half_delay = 196;
            tx_full_delay = 392;
            rx_delay = 294;
            rx_timeout = 392;
            break;
        }
        case DIRECT_IO_LATENCY_2:
        {
            tx_half_delay = 96;
            tx_full_delay = 192;
            rx_delay = 144;
            rx_timeout = 192;
            break;
        }
        case DIRECT_IO_LATENCY_3:
        {
            tx_half_delay = 46;
            tx_full_delay = 92;
            rx_delay = 69;
            rx_timeout = 92;
            break;
        }
        case DIRECT_IO_LATENCY_4:
        {
            tx_half_delay = 27;
            tx_full_delay = 42;
            rx_delay = 27;
            rx_timeout = 42;
            break;
        }
        case DIRECT_IO_LATENCY_5:
        {
            tx_half_delay = 14;
            tx_full_delay = 24;
            rx_delay = 14;
            rx_timeout = 24;
            break;
        }
        case DIRECT_IO_LATENCY_6:
        {
            tx_half_delay = 10;
            tx_full_delay = 16;
            rx_delay = 10;
            rx_timeout = 16;
            break;
        }
        case DIRECT_IO_LATENCY_7:
        {
            tx_half_delay = 5;
            tx_full_delay = 12;
            rx_delay = 1;
            rx_timeout = 12;
            break;
        }
    }
}

/*
 * @ brief: Сбрасывает внешнее прерывание
 * @ param: Нет
 * @ ret:   Нет
 */
void pru_conf_int_clear(void)
{
    CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
}
