 #ifndef _IO_H_
 #define _IO_H_

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>

#define SBNI0_RX            "/dev/sbni0/sbni-rx"
#define SBNI0_TX            "/dev/sbni0/sbni-tx"
#define SBNI1_RX            "/dev/sbni1/sbni-rx"
#define SBNI1_TX            "/dev/sbni1/sbni-tx"
#define SBNICTL             "/proc/sbni"
#define DEVPATH_LEN         sizeof(SBNI0_RX)

#define F_MODE              O_RDWR | O_CREAT | O_SYNC
#define F_PERM              S_IRWXU | S_IRWXG | S_IRWXO

struct sbnictl_struct
{
    bool is_running;
    uint8_t reserved[12];
};

int sbni_io_init(uint8_t num);
void sbni_io_deinit(uint8_t num);
size_t sbni_io_read(uint8_t num, uint8_t *buf);
void sbni_io_write(uint8_t num, uint8_t *data, size_t len);

extern void *sbnictl_map

#endif