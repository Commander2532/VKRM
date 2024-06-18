#include "main.h"
#include "prumsg.h"
#include "errors.h"
#include "threading.h"
#include "io.h"

const char *short_options = "dks::g:h";
const char *help_str = "Usage:\n-d or --daemon - starts sbni daemon\n-k or --kill - stops sbni daemon\n-s [ifnum] [speed] or --set-speed - configures speed of specific sbni interface\n-g or --get-speed [ifnum] - returns current speed of a specific sbni interface\n-h or --help - shows this help\n\n";

const struct option long_options[] =
{
    {"daemon",         no_argument, NULL, 'd'},
    {"kill",          no_argument, NULL, 'k'},
    {"set-speed",     required_argument, NULL, 's'},
    {"get-speed",     required_argument, NULL, 'g'},
    {"help",          no_argument, NULL, 'h'}
};

int main(int argc, char **argv)
{
    int ret, option;
    int option_index = 0;
    pid_t proc_pid;

    pru_memmap_init();
    while ((option = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (option)
        {
            case 'd':
                ret = sbni_interface_open(SBNI_0);
                if (ret)
                {
                    perror("sbni: unable to open sbni0");
                    return -1;
                }
                ret = sbni_interface_open(SBNI_1);
                if (ret) 
                {
                    sbni_interface_close(SBNI_0);
                    perror("sbni: unable to open sbni1");
                    return -1;
                }
                proc_pid = fork();
                if (proc_pid) return 0;
                goto daemon;
                break;
            case 'k':
                sbni_interface_close(SBNI_1);
                sbni_interface_close(SBNI_0);
                break;
            case 's':
                if (optarg)
                {
                    if (optarg[0] < '0' || optarg[0] > '1')
                    {
                        printf("sbni: invalid interface number, should be 0 or 1\n");
                        return -2;
                    }
                    if (optarg[1] < '0' || optarg[1] > '3')
                    {
                        printf("sbni: invalid speed value, should be in range of [0..3]\n");
                        return -2;
                    }
                    sbni_speed_set(atoi(&optarg[0]), atoi(&optarg[1]));
                }
                break;
            case 'g':
                if (optarg[0] < '0' || optarg[0] > '1')
                {
                    printf("sbni: invalid interface number, should be 0 or 1\n");
                    return -2;
                }
                printf("sbni: current sbni%d speed is '%d'\n", atoi(&optarg[0]), sbni_speed_get(atoi(&optarg[0])));
                break;
            case 'h':
                printf("%s", help_str);
                break;
            default:
                printf("%s", help_str);
        }
    }

    return 0;

daemon:
    sbnictl->cmd_stop= false;
    bool *cmd_stop = &sbnictl->cmd_stop;
    while(!cmd_stop) {}

kill_daemon:
    sbnictl->cmd_stop = true;
}

int sbni_interface_open(uint8_t num)
{
    int ret;

    ret = sbni_threads_create(num);
    
    if (ret)
        return ret;

    ret = sbni_io_init(num);

    return ret;
}

void sbni_interface_close(uint8_t num)
{
    sbni_threads_destroy(num);
    sbni_io_deinit(num);
    return;
}

void sbni_speed_set(int num, int speed)
{
    pru_speed_set((uint8_t)num, (uint8_t)speed);
}

int sbni_speed_get(int num)
{
    return (pru_speed_get((uint8_t)num));
}