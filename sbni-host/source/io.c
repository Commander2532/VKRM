#include "io.h"
#include "main.h"
#include "prumsg.h"

int iofd[2][2];
int sbnictl_fd;
void *sbnictl_addr;
struct sbnictl_map *sbnictl;

int sbni_io_init(uint8_t num)
{
    int ret;
    char filepath[2][DEVPATH_LEN];

    sbnictl_fd = open(SBNICTL, O_CREAT | O_RDWR);
    sbnictl_addr = mmap(NULL, sizeof(struct sbnictl_map), PROT_READ | PROT_WRITE, MAP_SHARED, sbnictl, 0);
    sbnictl = (struct sbnictl_map *)sbnictl_addr;

    switch (num)
    {
    case SBNI_0:
        ret = mkdir("/dev/sbni0", S_IRWXU | S_IRWXG | S_IRWXO);
        if (ret < 0 && errno != EEXIST)
        strncpy(filepath[0], SBNI0_RX, strlen(SBNI0_RX));
        strncpy(filepath[1], SBNI0_TX, strlen(SBNI0_TX));
            return errno;
        break;
    case SBNI_1:
        ret = mkdir("/dev/sbni1", S_IRWXU | S_IRWXG | S_IRWXO);
        if (ret < 0 && errno != EEXIST)
            return errno;
        strncpy(filepath[0], SBNI1_RX, strlen(SBNI1_RX));
        strncpy(filepath[1], SBNI1_TX, strlen(SBNI1_TX));
        break;
    }

    for (int i = 0; i < 2; i++)
    {
        truncate(filepath[i], 0);
        iofd[num][i] = open(filepath[i], F_MODE, F_PERM);
    }
    if (iofd[num][0] < 0 || iofd[num][1] < 0)
        return errno;

    return 0;
}

void sbni_io_deinit(uint8_t num)
{
    for (int i = 0; i < 2; i++)
        close(iofd[num][i]);
    close(sbnictl);
    return;
}

size_t sbni_io_read(uint8_t num, uint8_t *buf)
{
    int len;

    lseek(iofd[num][1], 0, SEEK_SET);
    len = read(iofd[num][1], buf, MAX_BUFFER_SIZE);
    
    if (len < 0)
        return errno;

    return len;
}

void sbni_io_write(uint8_t num, uint8_t *data, size_t len)
{
    if (len == 0) return;

    lseek(iofd[num][0], 0, SEEK_SET);
    write(iofd[num][0], data, len);

    return;
}
