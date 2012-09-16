#include <linux/version.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) )
#include <net/net_namespace.h>
#endif

MODULE_DESCRIPTION("Port Mirroring. Copy traffic from specified interfaces to MIRROR port.");
MODULE_AUTHOR("CU - platinum, http://weibo.com/bjpt @°×½ð-PT");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");

#define MAX_PORTS	32      /* Max number of the ports that we can capture. */

#define DEBUG		0
#if DEBUG
#define debugK printk
#else
#define debugK(format, args...)
#endif

static __u32 ports_c;
static __u16 ports[MAX_PORTS];
struct net_device *mirror;
struct net_device *ports_dev[MAX_PORTS] = { NULL };
module_param_array(ports, ushort, &ports_c, 0400);
MODULE_PARM_DESC(ports, " ports=[1,2,3,0] (Mirror packets from eth1 - eth3 to eth0)");
__read_mostly __u32 ifindex_bits = 0;

/* Check whether the packets came from the interface we cared. */
int inline
is_ports (struct net_device *dev)
{
        __u32 ifindex_bit = 1 << dev->ifindex;

        if (ifindex_bits & ifindex_bit)
                return 1;

        return 0;
}

/* Free all the ports we hold. */
void
free_ports (void)
{
        int i;

        for (i=0; i<32; i++) {
                if (ports_dev[i]) {
                        debugK ("free %s\n", ports_dev[i]->name);
                        dev_put(ports_dev[i]);
                }
        }
}

int
mirror_func (struct sk_buff *skb,
             struct net_device *dev,
             struct packet_type *pt,
             struct net_device *orig_dev)
{
        struct sk_buff *nskb;

        if (skb_shared(skb) && is_ports(dev)) {
                nskb = skb_clone(skb, GFP_ATOMIC);
                if (!nskb)
                        goto out;
                nskb->dev = mirror;
                nskb->len += nskb->mac_len;
                nskb->data -= nskb->mac_len;
                dev_queue_xmit(nskb);
        }
out:
        kfree_skb (skb);
        return NET_RX_SUCCESS;
}

static struct
packet_type mirror_proto = {
        /* Capture all protocols. */
        .type = __constant_htons(ETH_P_ALL),
        .func = mirror_func,
};

int
init_module (void)
{
        int ret;
        char interface[IFNAMSIZ];
        int i;
        struct net_device *dev;

        if (ports_c < 2) {
                debugK ("No ports found.\n");
                return -ENODEV;
        }

        for (i=0; i<ports_c-1; i++)
        {
                memset(interface, 0, sizeof(interface));
                if (ports[i] >= 32) {
                        debugK ("Wrong interface \"eth%d\", abort it.\n",
                                ports[i]);
                        continue;
                }

                snprintf(interface, sizeof(interface), "eth%d", ports[i]);
                debugK ("try eth%d\n", ports[i]);
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30) )
                dev = dev_get_by_name(interface);
#else
                dev = dev_get_by_name(&init_net, interface);
#endif
                if (!dev) {
                        debugK ("Can't found eth%d, abort it.\n", ports[i]);
                        continue;
                }

                ifindex_bits |= (1 << dev->ifindex);
                ports_dev[dev->ifindex] = dev;
        }

        /*
         * PT: Last interface for packets sent.
         * CU - platinum
         */
        snprintf(interface, sizeof(interface), "eth%d", ports[i]);
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30) )
        if ( (mirror = dev_get_by_name(interface)) )
#else
        if ( (mirror = dev_get_by_name(&init_net, interface)) )
#endif
        {
                dev_add_pack(&mirror_proto);
                printk("MIRROR module loaded on `%s'.\n", interface);
                ret = 0;
        } else {
                free_ports();
                ret = -ENODEV;
        }

        return ret;
}

void
cleanup_module(void)
{
        dev_remove_pack(&mirror_proto);
        dev_put(mirror);
        free_ports();
        printk("MIRROR module unloaded.\n");
}
