#include "sbni.h"
#include "crc32.h"
#include "interface.h"

extern HOST_MEM *shared_dram;
uint8_t packet_buffer[BUFFER_SIZE];

/*
 * @ brief: Подсчитывает контрольную сумму пакета
 * @ param: *frame - указатель на структуру с пакетом
 * @ ret:   Хэш CRC32
 */
uint32_t sbni_frame_crc(SBNI_FRAME* frame)
{
    uint32_t i;
    uint32_t result = CRC32_START_VALUE;

    for (i = 0; i < sizeof(SBNI_FRAME_SUBHEADER); i++)
        result=calc_crc32_value(result, frame->header->raw_subheader[i]);

    for (i = 0; i < (frame->header->subheader.length - SBNI_HLEN); i++)
        result=calc_crc32_value(result, frame->data[i]);

    return (result^CRC32_START_VALUE);
}

/*
 * @ brief: Обновляет поле в пакете для контрольной суммы
 * @ param: *frame - указатель на структуру с пакетом
 * @ ret:   Нет
 */
void sbni_update_crc(SBNI_FRAME* frame)
{
    *(frame->crc) = sbni_frame_crc(frame);
}

/*
 * @ brief: Проверяет принятый кадр SBNI на ошибки и извлекает из него
 *          данные
 * @ param: *dst_buffer - указатель на буфер, куда положить извлеченные данные
 * @ param: *src_buffer - указатель на буфер с принятым кадром SBNI
 * @ ret:   Статус кадра
 */
int8_t sbni_frame_get(uint8_t *dst_buffer, SBNI_FRAME *frame, uint16_t buffer_size)
{
    int16_t data_size;

    if (buffer_size < SBNI_MIN_FRAME_SIZE || buffer_size > SBNI_MAX_FRAME_SIZE)
        return SBNI_LENGTH_ERRORS;

    if (frame->header->marker != SBNI_MARKER)
        return SBNI_MARKER_ERRORS;

    data_size = frame->header->subheader.length - SBNI_HLEN;

    if (data_size < 0 || data_size > SBNI_MAX_DATA_SIZE)
        return SBNI_LENGTH_ERRORS;

    uint32_t calc_crc = sbni_frame_crc(frame);
    uint32_t recv_crc = *(frame->crc);

    if (recv_crc != calc_crc)
        return SBNI_CRC_ERRORS;

    if (data_size)
    {
        memcpy(dst_buffer, frame->data, data_size);
        return SBNI_FRAME_VALID;
    }
    return SBNI_FRAME_SERVICE;
}

/*
 * @ brief: Инкапсулирует данные в кадр SBNI
 * @ param: *frame - указатель на структуру кадра
 * @ param: number - номер кадра (на будущее)
 * @ param: *src_buffer - указатель на буфер с полезными данными
 * @ param: data_size - размер массива полезных данных
 * @ ret:   Нет
 */
void sbni_frame_set(SBNI_FRAME *frame, uint8_t number, uint8_t* src_data, uint16_t data_size)
{
    if (data_size <= SBNI_MAX_DATA_SIZE)
    {
        frame->header->marker               = SBNI_MARKER;
        frame->header->subheader.length     = data_size + SBNI_HLEN;
        frame->header->subheader.unused     = 0;
        frame->header->subheader.repeat     = 0;
        frame->header->subheader.first      = 1;
        frame->header->subheader.reserved   = 0;
        frame->header->subheader.number     = number;

        if (src_data && data_size)
            memcpy(frame->data, src_data, data_size);

        sbni_update_crc(frame);
    }
}

/*
 * @ brief: Формирует пакет на передачу
 * @ param: *dst_buffer - указатель на буфер, куда положить сформированный пакет
 * @ param: *src_buffer - указатель на буфер с полезными данными
 * @ param: data_size - размер массива полезных данных
 * @ ret:   Нет
 */
void sbni_packet_form(uint8_t *dst_buffer, uint8_t *src_buffer, uint16_t data_size)
{
    SBNI_FRAME frame;

    frame.header        = (SBNI_FRAME_HEADER *)&dst_buffer[0];
    frame.data          = (uint8_t *)&dst_buffer[SBNI_HLEN];
    frame.crc           = (uint32_t *)&dst_buffer[data_size + SBNI_HLEN];

    sbni_frame_set(&frame, 0, src_buffer, data_size);
}

/*
 * @ brief: Извлекает данные из принятого пакета
 * @ param: *dst_buffer - указатель на буфер, куда положить полезные данные
 * @ param: *src_buffer - указатель на буфер с принятым пакетом
 * @ param: data_size - размер принятого пакета
 * @ ret:   Статус пакета
 */
int8_t sbni_packet_parse(uint8_t *dst_buffer, uint8_t *src_buffer, uint16_t buffer_size)
{
    SBNI_FRAME frame;

    frame.header        = (SBNI_FRAME_HEADER *)&src_buffer[0];
    frame.data          = (uint8_t *)&src_buffer[SBNI_HLEN];
    frame.crc           = (uint32_t *)&src_buffer[buffer_size - SBNI_CRC_LEN];

    int8_t result = sbni_frame_get(dst_buffer, &frame, buffer_size);

    return result;
}

/*
 * @ brief: Формирует кадр SBNI и отправляет его
 * @ param: *src_buffer - указатель на буфер с данными для отправки
 * @ param: data_size - размер принятого пакета
 * @ ret:   Нет
 */
void sbni_data_send(uint8_t *src_buffer, uint16_t data_size)
{
    int8_t result = SBNI_TX_ERR;

    if (data_size <= SBNI_MAX_DATA_SIZE)
    {
        uint16_t packet_length = data_size + SBNI_HLEN + SBNI_CRC_LEN;
        sbni_packet_form(packet_buffer, src_buffer, data_size);
        pru_interface_data_send(packet_buffer, packet_length);
        result = SBNI_TX_OK;
    }
    pru_interface_stats_update(shared_dram, result);
}

/*
 * @ brief: Принимает кадр SBNI и извлекает из него данные
 * @ param: *dst_buffer - указатель на буфер, куда положить принятые данные
 * @ ret:   Размер принятых данных
 */
uint16_t sbni_data_receive(uint8_t *dst_buffer)
{
    uint16_t bytes_received = 0, data_size = 0;
    int8_t result = SBNI_LENGTH_ERRORS;

    bytes_received = pru_interface_data_read(packet_buffer);
    result = sbni_packet_parse(dst_buffer, packet_buffer, bytes_received);
    pru_interface_stats_update(shared_dram, result);

    if (result == SBNI_FRAME_VALID)
        data_size = bytes_received - SBNI_HLEN - SBNI_CRC_LEN;

    if (result == SBNI_FRAME_SERVICE)
        __NOP; // На будущее

    return data_size;
}
