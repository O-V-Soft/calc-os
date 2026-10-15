#include <net_dev.h>

#define MAX_NET_DEVICES 4

static struct net_device* registered_devices[MAX_NET_DEVICES];
static int net_device_count = 0;

void register_net_device(struct net_device* dev) {
    if (net_device_count < MAX_NET_DEVICES && dev != 0) {
        registered_devices[net_device_count++] = dev;
    }
}

struct net_device* get_primary_net_device() {
    if (net_device_count > 0) {
        return registered_devices[0]; 
    }
    return 0;
}
