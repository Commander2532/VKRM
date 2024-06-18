#ifndef _SBNI_GPIO_H_
#define _SBNI_GPIO_H_

#ifdef PRU1

#define TX_P_NUM                    21
#define TX_N_NUM                    19
#define RX_NUM                      18
#define CURRENT_PRU                 0x01

#else

#define TX_P_NUM                    24
#define TX_N_NUM                    22
#define RX_NUM                      23
#define CURRENT_PRU                 0x00

#endif

#define SBNI_TX_P                   (1<<TX_P_NUM)
#define SBNI_TX_N                   (1<<TX_N_NUM)
#define SBNI_RX                     (1<<RX_NUM)

#endif
