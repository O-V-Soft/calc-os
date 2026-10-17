#include <net_dev.h>
#include <net.h>

void net_receive(struct net_device* dev, const uint8_t* raw_packet, uint16_t len) {
    if (len < sizeof(struct eth_header)) return;

    struct eth_header* eth = (struct eth_header*)raw_packet;
    uint16_t ethertype = __builtin_bswap16(eth->ethertype);

    if (ethertype == 0x0806) {
        arp_receive(dev, raw_packet + sizeof(struct eth_header), len - sizeof(struct eth_header));
    }

    if (ethertype == 0x0800) { 
        ip_receive(dev, raw_packet + sizeof(struct eth_header), len - sizeof(struct eth_header));
    }
}

int net_ping(struct net_device* dev) {
    const char* msg = "CalcOS Ping";
    return icmp_send(dev, TARGET_IP, 8, 0, (const uint8_t*)msg, 10);
}
