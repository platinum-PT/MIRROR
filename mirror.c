#include <linux/version.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/sysrq.h>
#include <linux/smp.h>
#include <linux/netpoll.h>
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

struct net_device *mirror;
struct net_device *ports_dev[MAX_PORTS] = { NULL };
static char config[256];
module_param_string(mirror, config, 256, 0);
MODULE_PARM_DESC(mirror, " mirror=eth1[/eth2/eth3/...]@eth0 "
                "(Mirror eth1[and eth2 and eth3] traffic to eth0)");
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
                        printk("Remove data port: %s\n", ports_dev[i]->name);
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
option_setup (char *opt)
{
        char *from, *to, *cur, tmp;
        struct net_device *dev;
        int count = 0;

        printk ("args: %s\n", opt);

        /* Get mirror port. */
        if ((to = strchr(opt, '@')) == NULL)
                return -EINVAL;
        *to = '\0';
        to++;

        /* Get data ports. */
        from = opt;
        for (cur=opt; cur<=(opt+strlen(opt)); cur++) {
                if (*cur == '/' ||
                    *cur == '@' ||
                    *cur == ',' ||
                    *cur == 0x00)
                {
                        /* FIXME: why I must change it back, otherwise loop would stopped. */
                        tmp = *cur;
                        *cur = 0x00;
                        if (!strcmp(to, from)) {
                                printk ("%s is mirror port already.\n", to);
                                free_ports();
                                return -EBUSY;
                        }
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30) )
                        dev = dev_get_by_name(from);
#else
                        dev = dev_get_by_name(&init_net, from);
#endif
                        /* Set data ports. */
                        if (!dev) {
                                printk ("Cannot find data port: %s\n", from);
                                free_ports();
                                return -ENODEV;
                        }
                        printk("Set data port: %s\n", dev->name);
                        ports_dev[count++] = dev;
                        ifindex_bits |= (1 << dev->ifindex);
                        from = cur + 1;
                        /* FIXME: why I must change it back, otherwise loop would stopped. */
                        *cur = tmp;
                }
        }

        /* Set mirror port. */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30) )
        mirror = dev_get_by_name(to);
#else
        mirror = dev_get_by_name(&init_net, to);
#endif
        if (!mirror) {
                printk (KERN_ERR "Cannot find mirror port: %s\n", to);
                return -ENODEV;
        }
        printk("Set mirror port: %s\n", mirror->name);
        dev_add_pack(&mirror_proto);

        return 0;
}

int
init_module (void)
{
        int ret = 0;

        /* Learn from netconsole. */
        if (strlen(config)) {
                ret = option_setup(config);
                if (ret)
                        return ret;
        }

        printk("MIRROR module loaded.\n");
        return ret;
}

void
cleanup_module(void)
{
        dev_remove_pack(&mirror_proto);
        printk("Remove mirror port: %s\n", mirror->name);
        dev_put(mirror);
        free_ports();
        printk("MIRROR module unloaded.\n");
}
