#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/err.h>

#include <linux/remoteproc.h>
#include <linux/rpmsg.h>
#include <linux/pruss.h>
#include <asm/irq.h>

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netdev_features.h>
#include <linux/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/jiffies.h>

#define PRUMEM_DRAM_0                           (u32)0x4a310000
#define PRUMEM_DRAM_1                           (u32)0x4a311800
#define PRUMEM_SIZE                             (u32)0x1800

#define PRU0									0
#define PRU1									1

/* In Linux pru0 and pru1 are called remoteproc1 and remoteproc2 respectively */
#define REMOTEPROC1_PHANDLE                     0x2ca
#define REMOTEPROC2_PHANDLE                     0x2cb
#define REMOTEPROC1_FIRMWARE					"pru0-sbni-firmware.out"
#define REMOTEPROC2_FIRMWARE					"pru1-sbni-firmware.out"
#define REMOTEPROC1_RPMSG_CHAN					"/dev/rpmsg_pru30"
#define REMOTEPROC2_RPMSG_CHAN					"/dev/rpmsg_pru31"
#define REMOTEPROC1_IRQ_NUMBER					156
#define REMOTEPROC2_IRQ_NUMBER					158

#define SIOCDEVSBNI								SIOCDEVPRIVATE

#define SBNI_CMD_SET_SPEED						0
#define SBNI_CMD_GET_SPEED						1

#define SBNI_MIN_MTU                            8
#define SBNI_MAX_MTU                            1012
#define SBNI_HEADER_LEN                         0       // Handled by PRU
#define SBNI_TX_QUEUE_LEN                       100
#define SBNI_BUF_SIZE	                        2048

#define SBNI_TX_REQUEST							1
#define SBNI_SET_SPEED							2
#define SBNI_RPMSG_PROBE						0x55

#define SBNI_RX_PENDING							1
#define SBNI_TX_DONE							2
#define SBNI_SET_DONE							3
#define SBNI_RPMSG_RESPONSE						0xAA

struct net_device 	*sbni_netdev[2];
struct rproc 		*rproc_dev[2];
u32 				*memptr[2];

enum sbni_if_status
{
	SBNI_STATUS_FREE = 0,
	SBNI_STATUS_BUSY = 1
};

enum sbni_tx_status
{
	SBNI_TX_OK = 0,
	SBNI_TX_ERROR = 1,
	SBNI_TX_BUSY = 2
};

/* interrupt handler */
//static irqreturn_t sbni_irq_handler(int irq, void *dev_id);

/* netdev methods */
static inline enum sbni_tx_status sbni_hw_write_data(struct net_device *dev, u8 *data, int datalen);
static netdev_tx_t sbni_tx(struct sk_buff *skb, struct net_device *dev);
static void sbni_tx_timeout(struct net_device *netdev);
//static void sbni_rx(struct net_device *dev, struct sbni_packet *pkt);
struct net_device_stats* sbni_get_stats(struct net_device *dev);
static int sbni_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
static int sbni_open(struct net_device *dev);
static int sbni_close(struct net_device *dev);

/* RPMSG prototype functions */
static int sbni_rpmsg_callback(struct rpmsg_device *rpdev, void *data, int len, void *priv, u32 src);
static int sbni_rpmsg_probe(struct rpmsg_device *rpdev);
static void sbni_rpmsg_remove(struct rpmsg_device *rpdev);

/* RPMSG framework implementation */

static struct rpmsg_device_id rpmsg_sbni_driver_id_table[] = 
{
	{ .name	= "rpmsg-sbni-client" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_sbni_driver_id_table);

static struct rpmsg_driver rpmsg_sbni_drv = 
{
	.drv.name	= KBUILD_MODNAME,
	.drv.owner  = THIS_MODULE,
	.id_table	= rpmsg_sbni_driver_id_table,
	.probe		= sbni_rpmsg_probe,
	.callback	= sbni_rpmsg_callback,
	.remove		= sbni_rpmsg_remove,
};

static const struct net_device_ops sbni_netdev_ops =
{
    .ndo_open               = sbni_open,
    .ndo_stop               = sbni_close,
    .ndo_start_xmit         = sbni_tx,
    .ndo_tx_timeout         = sbni_tx_timeout,
    .ndo_get_stats          = sbni_get_stats,
    .ndo_do_ioctl           = sbni_do_ioctl
};

struct sbni_ioctl
{
	u8 cmd;
	u8 sbni_speed;
};

struct ifstat
{
	u64 tx_packets;
    u64 rx_packets;
    u64 sbni_len_errors;
    u64 sbni_marker_errors;
    u64 sbni_crc_errors;
};

struct sbni_packet
{
	int datalen;
	u8  *data;
};

struct sbni_hw
{
	u32 memaddr;
	u32 *memptr;
	u32 irq;
};

enum status
{
	PRU_STATUS_FREE = 0,
	PRU_STATUS_BUSY = 1
};

/* Memory layout of pru shared dram */
struct sbni_pru_mmap
{
    u8 rx_buffer[SBNI_BUF_SIZE];
    u8 tx_buffer[SBNI_BUF_SIZE];
    u16 rx_bytes;
    u16 tx_bytes;
    struct ifstat statistics;
    u8   sbni_speed;
	u8   evt_from_pru;
    u8   evt_to_pru;
	enum status pru_state;
    u8   mac_addr[6];
    u8   reserved[7];
} __attribute__((__packed__));

struct sbni_priv
{
	struct rpmsg_device *rpdev;
    struct net_device_stats stats;
    int status;
	struct sbni_pru_mmap *hw_memptr;
    struct sbni_packet *ppool;
    struct sbni_packet *rx_queue;
    int rx_int_enabled;
    int tx_packetlen;
    u8 *tx_packetdata;
    struct sk_buff *skb;
    spinlock_t lock;
};

struct net_device_stats* 
sbni_get_stats(struct net_device *dev)
{
	return;
}

inline void
sbni_rproc_notify(struct net_device *dev, u8 event)
{
	struct sbni_priv *priv = netdev_priv(dev);
	struct sbni_pru_mmap *pru_mmap = priv->hw_memptr;
	
	pru_mmap->evt_to_pru = event;
	
	//send interrupt to pru
	
	return;
}

static inline enum sbni_tx_status
sbni_hw_write_data(struct net_device *dev, u8 *data, int datalen)
{
	struct sbni_priv *priv = netdev_priv(dev);
	struct sbni_pru_mmap *pru_mmap = priv->hw_memptr;
	
	if (pru_mmap->pru_state == SBNI_STATUS_FREE)
	{
		memcpy(pru_mmap->tx_buffer, data, datalen);
		pru_mmap->tx_bytes = datalen;
		sbni_rproc_notify(dev, SBNI_TX_REQUEST);
	}
	
	return SBNI_TX_BUSY;
}

static void
sbni_hw_read_data(struct net_device *dev, struct sbni_packet *pkt)
{
	struct sbni_priv *priv = netdev_priv(dev);
	struct sbni_pru_mmap *pru_mmap = priv->hw_memptr;
	
	int rx_bytes = pru_mmap->rx_bytes;
	
	if (rx_bytes)
		memcpy(pkt->data, pru_mmap->rx_buffer, rx_bytes);
	
	pkt->datalen = rx_bytes;
	
	return;
}

/* Generic transmit function */
static int
sbni_tx(struct sk_buff *skb, struct net_device *dev)
{
    int len;
    u8 *data;
    struct sbni_priv *priv = netdev_priv(dev);

    data = skb->data;
    len = skb->len;

	dev_trans_start(dev);

    priv->skb = skb;

    if (!sbni_hw_write_data(dev, data, len))
		return NETDEV_TX_OK;

    return NETDEV_TX_BUSY;
}

/* Transmit failures handler */ 
static void
sbni_tx_timeout(struct net_device *netdev)
{
    return;
}

/* Recieve function */
static void sbni_rx(struct net_device *dev, struct sbni_packet *pkt)
{
    struct sk_buff *skb;
    struct sbni_priv *priv = netdev_priv(dev);
	struct sbni_pru_mmap *pru_mmap = priv->hw_memptr;
	
	sbni_hw_read_data(dev, pkt);
	
	if (unlikely(!pkt->datalen))
	{
		if (printk_ratelimit())
			printk(KERN_NOTICE "sbni_pruss: received zero length packet (how?)\n");
		goto out;
	}
	
	skb = dev_alloc_skb(pkt->datalen + 2);
	
    if (!skb)
    {
		if (printk_ratelimit())
			printk(KERN_NOTICE "sbni_pruss: rx packet dropped due to memory shortage\n");
		priv->stats.rx_dropped++;
        goto out;
    }

    memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);

    skb->dev = dev;
    skb->protocol = eth_type_trans(skb, dev);
    skb->ip_summed = CHECKSUM_UNNECESSARY;

    priv->stats.rx_packets++;
    priv->stats.rx_bytes += pkt->datalen;
	
    netif_rx(skb);

out:
    return;
}

/* IRQ handler */
/*
static irqreturn_t
sbni_irq_handler(int irq, void *dev_id)
{
	u8 event;
	struct sbni_priv *priv;
	struct sbni_packet *pkt = NULL;
	
	struct net_device *dev = (struct net_device *)dev_id;
	
	if (!dev) return;
	
	priv = netdev_priv(dev);
	
	
	spin_lock(&priv->lock);
	event = pru_mmap->evt_from_pru;
	
	
	
	spin_unlock(&priv->lock);
	//if (pkt) sbni_release_buffer(pkt);
	
	return IRQ_HANDLED;
}
*/

/* SBNI ioctl implementation */
static int
sbni_do_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct sbni_priv *priv = netdev_priv(dev);
    struct sbni_ioctl *ioctl = (struct sbni_ioctl *) &rq->ifr_ifru;
    struct sbni_pru_mmap *pru_mmap = priv->hw_memptr;
	
	if (cmd != SIOCDEVSBNI)
		return -EOPNOTSUPP;
	
	if (in_compat_syscall())
		return -EOPNOTSUPP;
	
    switch(ioctl->cmd)
    {
	case SBNI_CMD_SET_SPEED:
		if (ioctl->sbni_speed < 0 || ioctl->sbni_speed > 6)
			return -EINVAL;
        pru_mmap->sbni_speed = ioctl->sbni_speed;
		sbni_rproc_notify(dev, SBNI_SET_SPEED);
        break;

    case SBNI_CMD_GET_SPEED:
		ioctl->sbni_speed = pru_mmap->sbni_speed;
        break;
		
    default:
		return -EOPNOTSUPP;
    }

    return 0;
}

/* Interface open function */
static int
sbni_open(struct net_device *dev)
{
	int err = 0;
	u32 memaddr;
	u32 **mmap_ptr;
	u32 irqn;
	struct sbni_priv *priv = netdev_priv(dev);
	
	if (dev == sbni_netdev[0])
	{
		memaddr = PRUMEM_DRAM_0;
		mmap_ptr = &memptr[0];
		irqn = REMOTEPROC1_IRQ_NUMBER;
	}
	else if (dev == sbni_netdev[1])
	{
		memaddr = PRUMEM_DRAM_1;
		mmap_ptr = &memptr[1];
		irqn = REMOTEPROC2_IRQ_NUMBER;
	}
	else return -ENXIO;
	
	printk(KERN_INFO "sbni_pruss: mapping mem region 0x%X\n", PRUMEM_DRAM_0);
		
	if (!request_mem_region(memaddr, PRUMEM_SIZE, dev->name))
	{
		err = -ENOMEM;
		printk(KERN_ERR "sbni_pruss: unable to request memory region 0x%X, size 0x%X\n", memaddr, PRUMEM_SIZE);
		goto out;
	}
	
	if (!(*mmap_ptr = (u32 *)ioremap(memaddr, PRUMEM_SIZE)))
	{
		err = -EFAULT;
		printk(KERN_ERR "sbni_pruss: unable to map memory region\n");
		goto out_free_mem;
	}
	
	//printk(KERN_INFO "sbni_pruss: freeing default irq line\n");
	//free_irq(irqn, &(priv->rproc->dev));
	/*
	if (request_irq(irqn, sbni_irq_handler, IRQF_ONESHOT, dev->name, (void *)(sbni_irq_handler)))
	{
		err = -EFAULT;
		printk(KERN_ERR "sbni_pruss: unable to request irq line %d\n", irqn);
		goto out_unmap;
	}
	*/
	priv->hw_memptr = (struct sbni_pru_mmap *)(*mmap_ptr);
	
	memcpy(dev->dev_addr, "\0\0PRU0", 6);
	
	if (!err)
	{
		netif_start_queue(dev);
		printk(KERN_INFO "sbni_pruss: %s becomes online\n", dev->name);
	}
	
	return 0;

out_unmap:
	iounmap(*mmap_ptr);

out_free_mem:
	release_mem_region(memaddr, PRUMEM_SIZE);
out:
    return err;
}

/* Interface close function */
static int
sbni_close(struct net_device *dev)
{
	if (dev == sbni_netdev[0])
	{
		iounmap(memptr[0]);
		free_irq(REMOTEPROC1_IRQ_NUMBER, NULL);
		release_mem_region(PRUMEM_DRAM_0, PRUMEM_SIZE);
	}
	else if (dev == sbni_netdev[1])
	{
		iounmap(memptr[1]);
		free_irq(REMOTEPROC2_IRQ_NUMBER, NULL);
		release_mem_region(PRUMEM_DRAM_1, PRUMEM_SIZE);
	}
    netif_stop_queue(dev);
	printk(KERN_INFO "sbni_pruss: %s becomes offline\n", dev->name);
	
    return 0;
}

/* Helper function for configuring network features */
static void
sbni_netdev_setup(struct net_device *dev)
{
    dev->type               = ARPHRD_NONE;
    dev->hard_header_len    = SBNI_HEADER_LEN;
    dev->min_header_len     = SBNI_HEADER_LEN;
    dev->min_mtu            = SBNI_MIN_MTU;
    dev->max_mtu            = SBNI_MAX_MTU;
    dev->mtu                = SBNI_MAX_MTU;
    dev->addr_len           = SBNI_HEADER_LEN;
    dev->tx_queue_len       = SBNI_TX_QUEUE_LEN;

    dev->flags              = IFF_NOARP
							| IFF_POINTOPOINT;

	dev->features           = NETIF_F_VLAN_CHALLENGED
                            | NETIF_F_HW_CSUM;

    //dev->header_ops = &sbni_header_ops;
    dev->netdev_ops  		= &sbni_netdev_ops;
}

static int 
sbni_rpmsg_callback(struct rpmsg_device *rpdev, void *data, int len, void *priv, u32 src)
{
	int ret, pruid, event;
	struct net_device *dev = NULL;
	struct sbni_packet *pkt = NULL;
	struct sbni_priv *sbpriv;
	struct sbni_pru_mmap *pru_mmap;
	
	memcpy(&pruid, data, 1);
	memcpy(&event, data, 1);
	
	if (pruid == PRU0)
		dev = sbni_netdev[0];
	else if (pruid == PRU1)
		dev = sbni_netdev[1];
	else goto out;
	
	sbpriv = netdev_priv(dev); 
	pru_mmap = sbpriv->hw_memptr;
	
	spin_lock(&sbpriv->lock);
	switch(event)
	{
	case SBNI_RX_PENDING:
		sbni_rx(dev, pkt);
		break;
	
	case SBNI_TX_DONE:
		sbpriv->stats.tx_packets++;
		sbpriv->stats.tx_bytes += sbpriv->tx_packetlen;
		dev_kfree_skb(sbpriv->skb);
		break;
	
	case SBNI_SET_DONE:
		if (printk_ratelimit())
			printk(KERN_NOTICE "sbni_pruss: changed sbni speed to %d", pru_mmap->sbni_speed);
		break;
		
	case SBNI_RPMSG_RESPONSE:
		printk(KERN_INFO "sbni_pruss: pru%d rpmsg endpoint probed successfully\n", pruid);
	}
	spin_unlock(&sbpriv->lock);
	
out:
	return 0;
}

static int sbni_rpmsg_probe(struct rpmsg_device *rpdev)
{
	int ret;
	u8 msg = SBNI_RPMSG_PROBE;

	dev_info(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n", rpdev->src, rpdev->dst);
	
	ret = rpmsg_send(rpdev->ept, &msg, 1);
	if (ret) 
	{
		dev_err(&rpdev->dev, "sbni_pruss: failed to probe rpmsg: %d\n", ret);
		return ret;
	}

	return 0;
}

static void sbni_rpmsg_remove(struct rpmsg_device *rpdev)
{
	dev_info(&rpdev->dev, "rpmsg sample client driver is removed\n");
}

/* Load module function */ 
static int __init
sbni_driver_init(void)
{
    int err;
	
    printk(KERN_INFO "sbni_pruss: loading sbni network driver\n");
    
	/* Get rproc device contexts from device tree */
    rproc_dev[0] = rproc_get_by_phandle(REMOTEPROC1_PHANDLE);
    rproc_dev[1] = rproc_get_by_phandle(REMOTEPROC2_PHANDLE);
    if (!rproc_dev[0] || !rproc_dev[1])
	{
		printk(KERN_ERR "sbni_pruss: invalid phandle %d for %s\n", (!rproc_dev[0] ? REMOTEPROC1_PHANDLE : REMOTEPROC2_PHANDLE), (!rproc_dev[0] ? "remoteproc1" : "remoteproc2"));
		err = -ENODEV;
		goto out;
	}

	printk(KERN_INFO "sbni_pruss: setting firmware\n");
	
	/* Set appropriate firmware file name for both rprocs */
	err = rproc_set_firmware(rproc_dev[0], REMOTEPROC1_FIRMWARE);
	if (err)
	{
		printk(KERN_ERR "sbni_pruss: firmware file \"%s\" not found\n", REMOTEPROC1_FIRMWARE);
		goto out;
	}
	err = rproc_set_firmware(rproc_dev[1], REMOTEPROC2_FIRMWARE);
	if (err)
	{
		printk(KERN_ERR "sbni_pruss: firmware file \"%s\" not found\n", REMOTEPROC2_FIRMWARE);
		goto out;
	}
	printk(KERN_INFO "sbni_pruss: booting up PRUs\n");
	
	/* Now boot up both rprocs */
    err = rproc_boot(rproc_dev[0]);
    if (err)
	{
		printk(KERN_ERR "sbni_pruss: unable to boot pru0\n");
		err = -ENODEV;
		goto out;
	}
    err = rproc_boot(rproc_dev[1]);
    if (err)
	{
		printk(KERN_ERR "sbni_pruss: unable to boot pru1\n");
		err = -ENODEV;
		goto out_stop_rproc1;
	}
	
	printk(KERN_INFO "sbni_pruss: registering rpmsg driver\n");
	err = register_rpmsg_driver(&rpmsg_sbni_drv);
	
	if (err)
	{
		printk(KERN_ERR "sbni_pruss: unable to register rpmsg driver\n");
		goto out_stop_rproc2;
	}
	
	/* Allocate and register new net devices */
    sbni_netdev[0] = alloc_netdev(sizeof(struct sbni_priv), "sbni0", NET_NAME_PREDICTABLE, sbni_netdev_setup);
    sbni_netdev[1] = alloc_netdev(sizeof(struct sbni_priv), "sbni1", NET_NAME_PREDICTABLE, sbni_netdev_setup);
    if (!sbni_netdev[0] || !sbni_netdev[1])
	{
		err = -ENOMEM;
		goto out_stop_rproc2;
	}
	
	err = register_netdev(sbni_netdev[0]);
	if (err)
		goto out_free_netdev;
	err = register_netdev(sbni_netdev[1]);
	if (err)
		goto out_free_netdev;
	
	return 0;
	
	/* Free netdev if it was allocated */
out_free_netdev:
    free_netdev(sbni_netdev[0]);
    free_netdev(sbni_netdev[1]);
	
	/* Stop both remoteproc2 and remoteproc1 (if remoteproc2 was running, it means remoteproc1 was running too) */
out_stop_rproc2:
	rproc_shutdown(rproc_dev[1]);

	/* Stop only remoteproc1 (in case remoteproc2 failed to boot) */
out_stop_rproc1:
	rproc_shutdown(rproc_dev[0]);
	
	/* Exit if there's nothing to stop or free */
out:
	printk(KERN_ERR "sbni_pruss: failed to load module\n");
    return err;
}

/* Unload module function */
static void __exit
sbni_driver_cleanup(void)
{
    int i;

    for (i = 0; i < 2; i++)
    {
		if (sbni_netdev[i])
        {
			unregister_netdev(sbni_netdev[i]);
            free_netdev(sbni_netdev[i]);
        }
    }

	unregister_rpmsg_driver(&rpmsg_sbni_drv);

    rproc_shutdown(rproc_dev[0]);
    rproc_shutdown(rproc_dev[1]);
	printk(KERN_INFO "sbni_pruss: sbni network driver unloaded\n");
	
    return;
}

module_init(sbni_driver_init);
module_exit(sbni_driver_cleanup);
//module_rpmsg_driver(rpmsg_sbni_client);

MODULE_DESCRIPTION("SBNI PRU-ICSS network driver");
MODULE_LICENSE("GPL");