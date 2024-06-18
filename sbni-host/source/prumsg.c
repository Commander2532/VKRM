#include "prumsg.h"
#include "errors.h"

void *dram_virtual_addr;
struct pollfd rpmsg_ch[2] = {0, 0};
int sbnictl_fd;
sbnictl_mmap *sbnictl;

PRU_SHARED_MEMORY *pru_shared_dram[2];

int pru_memmap_init(void)
{
    printf("sbni_debug: mapping memory file\n");
    int memfd = open(DEVICE_NAME_MEMORY, O_RDWR | O_SYNC);

    if (memfd < 0)
        return errno;

    dram_virtual_addr = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, SHARED_DRAM_ADDR);
    
    if (dram_virtual_addr == (void *) -1)
        return -ENOMEM;
	
	pru_shared_dram[PRU_0] = (PRU_SHARED_MEMORY *)(dram_virtual_addr);
    pru_shared_dram[PRU_1] = (PRU_SHARED_MEMORY *)(dram_virtual_addr + SHARED_DRAM_SIZE);
    close(memfd);

	printf("sbni_debug: mapping ctl file\n");
	int sbnictl_fd = open(SBNICTL_FILE, O_RDWR | O_SYNC | O_CREAT);
	sbnictl = (sbnictl_mmap *)mmap(NULL, sizeof(sbnictl_mmap), PROT_READ | PROT_WRITE, MAP_SHARED, sbnictl_fd, NULL);
	if (sbnictl == (void *) -1)
		return -ENODEV;

    return 0;
}

void pru_memmap_deinit(void)
{
    munmap(dram_virtual_addr, MAP_SIZE);
	munmap(sbnictl);
	close(sbnictl_fd);
}

int pru_firmware_set(uint8_t num, const char *file)
{
    int fd;

    switch(num)
    {
        case PRU_0:
            fd = open(RPROC1_FW_SET, O_WRONLY);
            break;
        case PRU_1:
            fd = open(RPROC2_FW_SET, O_WRONLY);
            break;
        default:
            return -EINVAL;
    }

    if (fd < 0) 
        return -EBADF;

    write(fd, file, strlen(file));
    close(fd);

    return 0;
}

void pru_stop(uint8_t num)
{
    int fd;

    switch (num)
    {
        case PRU_0:
            fd = open(RPROC1_CTL_FILE, O_WRONLY);
            break;
        case PRU_1:
            fd = open(RPROC2_CTL_FILE, O_WRONLY);
            break;
        default:
            return;
    }

    if (fd < 0) 
        return;

    write(fd, "stop", 4);
    sleep(1);

    return;
}

int pru_start(uint8_t num)
{
    int fd;
    char state[7] = {0};

    switch (num)
    {
        case PRU_0:
            if (pru_firmware_set(num, RPROC1_FW_NAME))
                return -ENODEV;
            fd = open(RPROC1_CTL_FILE, O_RDWR);
            break;
        case PRU_1: 
            if (pru_firmware_set(num, RPROC2_FW_NAME))
                return -ENODEV;
            fd = open(RPROC2_CTL_FILE, O_RDWR);
            break;
        default:
            return -EINVAL;
    }

    if (fd < 0)
        return -EBADF;
    
    read(fd, state, 7);

    if (!strncmp(state, "running", 7))
    {
        pru_stop(num);
        sleep(1);
    }

    write(fd, "start", 5);
    sleep(1);

    if (!strncmp(state, "offline", 7))
        return -ENOTRECOVERABLE;
    
    return 0;
}

int pru_rproc_init(uint8_t num)
{
    int ret = 0;

    ret = pru_start(num);
    if (ret)
        return ret;

    printf("sbni_debug: initializing pru rpmsg framework\n");
    switch (num)
    {
        case PRU_0:
            rpmsg_ch[num].fd = open(DEVICE_NAME_PRU0, O_RDWR);
            truncate(DEVICE_NAME_PRU0, 0);
            break;
        case PRU_1:
            rpmsg_ch[num].fd = open(DEVICE_NAME_PRU1, O_RDWR);
            truncate(DEVICE_NAME_PRU1, 0);
            break;
        default:
            return -ENODEV;
    }

    if (rpmsg_ch[num].fd < 0)
    {
        pru_stop(num);
        return -ENODEV;
    } 

    return 0;
}

void pru_rproc_deinit(uint8_t num)
{
    close(rpmsg_ch[num].fd);
    pru_stop(num);
    return;
}

void pru_speed_set(uint8_t num, uint8_t speed)
{
    pru_shared_dram[num]->sbni_speed = speed;
    pru_msg_event_set(num, TO_PRU_EVENT_SET_SPEED);
    return;
}

int pru_speed_get(uint8_t num)
{
    return pru_shared_dram[num]->sbni_speed;
}

void pru_msg_event_set(uint8_t num, uint8_t event)
{
    uint8_t message[2] = { };
    message[0] = num;
    message[1] = event;
    write(rpmsg_ch[num].fd, message, 2);
}

uint8_t pru_msg_event_get(uint8_t num)
{
    uint8_t message[2] = { };
    read(rpmsg_ch[num].fd, message, 2);
    return message[1];
}

void pru_tx_data_set(uint8_t num, uint8_t *data, uint16_t data_length)
{
    if (data_length >= 0 && data_length <= MAX_BUFFER_SIZE)
    {
        memcpy(pru_shared_dram[num]->tx_buffer, data, data_length);
        pru_shared_dram[num]->bytes_to_send = data_length;
    }
}

uint16_t pru_rx_data_get(uint8_t num, uint8_t *buffer)
{
    uint16_t bytes_received = pru_shared_dram[num]->bytes_received;
    if (bytes_received >= 0 && bytes_received <= MAX_BUFFER_SIZE)
        memcpy(buffer, pru_shared_dram[num]->rx_buffer, bytes_received);
    return bytes_received;
}
