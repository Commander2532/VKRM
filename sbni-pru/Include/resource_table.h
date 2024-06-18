#ifndef _RESOURCE_TABLE_H_
#define _RESOURCE_TABLE_H_

#include <stddef.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include "pru_virtio_ids.h"

#ifdef SELECT_PRU0
#define TO_ARM_HOST                     16
#define FROM_ARM_HOST                   17
#define PRU0_TO_ARM_CHANNEL             2
#define PRU0_FROM_ARM_CHANNEL           0
#define PRU1_TO_ARM_CHANNEL             HOST_UNUSED
#define PRU1_FROM_ARM_CHANNEL           HOST_UNUSED
#define CHAN_NAME                       "rpmsg-pru"
#define CHAN_PORT                       (signed int)30
#define TO_ARM_CHANNEL                  PRU0_TO_ARM_CHANNEL
#define FROM_ARM_CHANNEL                PRU0_FROM_ARM_CHANNEL
#endif
#ifdef SELECT_PRU1
#define TO_ARM_HOST                     18
#define FROM_ARM_HOST                   19
#define PRU0_TO_ARM_CHANNEL             HOST_UNUSED
#define PRU0_FROM_ARM_CHANNEL           HOST_UNUSED
#define PRU1_TO_ARM_CHANNEL             3
#define PRU1_FROM_ARM_CHANNEL           1
#define CHAN_NAME                       "rpmsg-pru"
#define CHAN_PORT                       (signed int)31
#define TO_ARM_CHANNEL                  PRU1_TO_ARM_CHANNEL
#define FROM_ARM_CHANNEL                PRU1_FROM_ARM_CHANNEL
#endif

#define VIRTIO_CONFIG_S_DRIVER_OK       4

#define PRU_RPMSG_VQ0_SIZE              16
#define PRU_RPMSG_VQ1_SIZE              16
#define VIRTIO_RPMSG_F_NS               0
#define RPMSG_PRU_C0_FEATURES           (1<<VIRTIO_RPMSG_F_NS)
#define HOST_UNUSED                     255

struct ch_map
{
    unsigned char to_arm_host;
    unsigned char to_arm_ch;
};

#pragma DATA_SECTION(pru_intc_map, ".pru_irq_map")
#pragma RETAIN(pru_intc_map)
struct ch_map pru_intc_map[] =
{
    {(unsigned char)TO_ARM_HOST,   (unsigned char)TO_ARM_CHANNEL},
    {(unsigned char)FROM_ARM_HOST, (unsigned char)FROM_ARM_CHANNEL},
};

struct my_resource_table
{
    struct resource_table    base;
    uint32_t                 offset[2];
    struct fw_rsc_vdev       rpmsg_vdev;
    struct fw_rsc_vdev_vring rpmsg_vring0;
    struct fw_rsc_vdev_vring rpmsg_vring1;
    struct fw_rsc_custom     pru_ints;
};

#pragma DATA_SECTION(resourceTable, ".resource_table")
#pragma RETAIN(resourceTable)
struct my_resource_table resourceTable =
{
    {
        (unsigned int)1,                    /* Resource table version: only version 1 is supported by the current driver */
        (unsigned int)2,                    /* number of entries in the table */
        (unsigned int)0, (unsigned int)0,   /* reserved, must be zero */
    },
    {
         offsetof(struct my_resource_table, rpmsg_vdev),
         offsetof(struct my_resource_table, pru_ints),
    },
    {
        (unsigned int) TYPE_VDEV,               //type
        (unsigned int) VIRTIO_ID_RPMSG,         //id
        (unsigned int) 0,                       //notifyid
        (unsigned int) RPMSG_PRU_C0_FEATURES,   //dfeatures
        (unsigned int) 0,                       //gfeatures
        (unsigned int) 0,                       //config_len
        (unsigned char)0,                       //status
        (unsigned char)2,                       //num_of_vrings, only two is supported
        { (unsigned char)0, (unsigned char)0 }, //reserved
    },
    {
        0,                                      //da, will be populated by host, can't pass it in
        16,                                     //align (bytes),
        PRU_RPMSG_VQ0_SIZE,                     //num of descriptors
        0,                                      //notifyid, will be populated, can't pass right now
        0                                       //reserved
    },
    {
        0,                                      //da, will be populated by host, can't pass it in
        16,                                     //align (bytes),
        PRU_RPMSG_VQ1_SIZE,                     //num of descriptors
        0,                                      //notifyid, will be populated, can't pass right now
        0                                       //reserved
    },
    {
        TYPE_CUSTOM,
        TYPE_PRU_INTS,
        sizeof(struct fw_rsc_custom_ints),
        {   // PRU_INTS version
            0x0000,
            //Channel-to-host mapping, 255 for unused
            PRU0_FROM_ARM_CHANNEL, PRU1_FROM_ARM_CHANNEL, PRU0_TO_ARM_CHANNEL, PRU1_TO_ARM_CHANNEL,
            HOST_UNUSED, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED,
            //Number of evts being mapped to channels
            (sizeof(pru_intc_map) / sizeof(struct ch_map)),
            //Pointer to the structure containing mapped events
            pru_intc_map,
        },
    },
};

#endif
