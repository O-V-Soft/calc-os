#ifndef NET_DEV_H
#define NET_DEV_H
#include <stdint.h>

struct net_device {
    char name[16];
    uint8_t mac_addr[6];
    
    void (*init)();
    int (*send_packet)(const uint8_t* data, uint16_t len);
    int (*poll_packet)(uint8_t* buffer, uint16_t max_len);
};

void register_net_device(struct net_device* dev);
struct net_device* get_primary_net_device();

#endif