#ifndef _CRC32_H_
#define _CRC32_H_

#define CRC32_START_VALUE  0xFFFFFFFF

unsigned int calc_crc32_value(unsigned int crc, unsigned char value);
unsigned int calc_crc32(unsigned char* buffer, unsigned int buffer_size);

#endif
