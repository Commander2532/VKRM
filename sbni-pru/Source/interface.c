#include "main.h"
#include "sbni.h"
#include "interface.h"
#include "asmfunc.h"

uint8_t  current_speed;
uint32_t rx_count;

int32_t tx_pointer = 0;
int32_t rx_pointer = 0;

uint8_t raw_rx_buffer[BUFFER_SIZE];

struct pru_rpmsg_transport transport;

/*
 * @ brief: Выполняет первоначальную настройку SBNI
 * @ param: Нет
 * @ ret:   Нет
 */
void pru_init_all(void)
{
    /* Инициализация регистров и транспортного слоя */
    pru_conf_sys_init(&transport);

    /* Инициализация выводов */
    pru_conf_reset_pins();

    /* Установка скорости по-умолчанию */
    pru_interface_speed_set(SBNI_SPEED_125);
}

/*
 * @ brief: Настраивает задержки в зависимости
 *          от выбранной скорости
 * @ param: new_speed - скорость SBNI
 * @ ret  : Нет
 */
void pru_interface_speed_set(unsigned char new_speed)
{
    uint8_t latency;
    switch(new_speed)
    {
        case SBNI_SPEED_125:
            latency = DIRECT_IO_LATENCY_0;
            break;
        case SBNI_SPEED_250:
            latency = DIRECT_IO_LATENCY_1;
            break;
        case SBNI_SPEED_500:
            latency = DIRECT_IO_LATENCY_2;
            break;
        case SBNI_SPEED_1000:
            latency = DIRECT_IO_LATENCY_3;
            break;
        case SBNI_SPEED_2000:
            latency = DIRECT_IO_LATENCY_4;
            break;
        case SBNI_SPEED_3125:
            latency = DIRECT_IO_LATENCY_5;
            break;
        case SBNI_SPEED_4166:
            latency = DIRECT_IO_LATENCY_6;
            break;
        default:
            latency = DIRECT_IO_LATENCY_0;
            break;
    }
    pru_conf_set_latency(latency);
    current_speed = new_speed;
}

/*
 * @ brief: Побайтово передаёт массив данных
 * @ param: *buffer - указатель на массив данных
 * @ param: buffer_size - размер массива
 * @ ret  : Нет
 */
void pru_interface_data_send(uint8_t* buffer, uint16_t buffer_size)
{
    uint8_t balance_zero_seq = 0;

    if (buffer_size <= BUFFER_SIZE)
    {
        tx_pointer = buffer_size - 1;
        __transmit_byte(balance_zero_seq);
        while (tx_pointer >= 0)
            __transmit_byte(buffer[tx_pointer--]);
    }
    pru_conf_reset_pins();
}

/*
 * @ brief: Читает данные из приёмного буфера и перемещает их в в буфер
 *          по переданному указателю
 * @ param: *buffer - указатель на буфер, куда сохранять данные
 * @ ret  : Количество принятых байт
 */
uint16_t pru_interface_data_read(uint8_t *buffer)
{
    uint16_t bytes_received = 0;
    if (rx_count > 1 && rx_count < BUFFER_SIZE)
    {
        int i;
        for (i = 0; i < rx_count; i++)
            buffer[i] = raw_rx_buffer[rx_count - i - 1];
        bytes_received = rx_count;
        memset(raw_rx_buffer, 0, BUFFER_SIZE);
    }
    rx_count = 0;
    rx_pointer = 0;
    return bytes_received;
}

/*
 * @ brief: Обновляет статистику пакетов
 * @ param: *shared_dram - указатель на структуру в разделяемой памяти
 * @ param: stats_entry - идентификатор записи в статистике
 * @ ret  : Нет
 */
void pru_interface_stats_update(HOST_MEM *shared_dram, int8_t stats_entry)
{
    switch(stats_entry)
    {
        case SBNI_PACKETS_SENT:
            shared_dram->stats.tx_packets++;
            break;
        case SBNI_PACKETS_RECEIVED:
            shared_dram->stats.rx_packets++;
            break;
        case SBNI_LENGTH_ERRORS:
            shared_dram->stats.sbni_len_errors++;
            break;
        case SBNI_MARKER_ERRORS:
            shared_dram->stats.sbni_marker_errors++;
            break;
        case SBNI_CRC_ERRORS:
            shared_dram->stats.sbni_crc_errors++;
            break;
    }
}

/*
 * @ brief: Отправляет событие к хосту
 * @ param: *rpmsg_payload - указатель на сообщение
 * @ param: rpmsg_len - длина сообщения
 * @ ret  : Нет
 */
void arm_host_message_send(unsigned char *rpmsg_payload, unsigned short rpmsg_length)
{
    pru_rpmsg_send(&transport, RPMSG_PRU_PORT, RPMSG_ARM_PORT, rpmsg_payload, rpmsg_length);
}

/*
 * @ brief: Обрабатывает сообщение от хоста
 * @ param: *shared_dram - указатель на структуру в разделяемой памяти
 * @ ret  : Нет
 */
void arm_host_message_handle(HOST_MEM *shared_dram)
{
    unsigned short src, dst, len;
    unsigned char payload;
    unsigned char event_to_host = 0;

    while (pru_rpmsg_receive(&transport, &src, &dst, &payload, &len) == PRU_RPMSG_SUCCESS)
    {
        unsigned char event = payload;
        switch(event)
        {
            case FROM_HOST_EVENT_TX_REQUEST:
            {
                sbni_data_send(shared_dram->tx_buffer, shared_dram->tx_bytes);
                memset(shared_dram->tx_buffer, 0, BUFFER_SIZE);
                shared_dram->tx_bytes = 0;
                event_to_host = TO_HOST_EVENT_TX_DONE;
                break;
            }
            case FROM_HOST_EVENT_SPEED_SET:
            {
                pru_interface_speed_set(shared_dram->sbni_speed);
                event_to_host = TO_HOST_EVENT_SPEED_SET;
                break;
            }
        }
        pru_rpmsg_send(&transport, dst, src, &event_to_host, 1);
    }
}
