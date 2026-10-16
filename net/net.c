#include <net_dev.h>
#include <net.h>

void ip_receive(struct net_device* dev, const uint8_t* packet, uint16_t len) {
    if (len < sizeof(struct ipv4_header)) {
        return; 
    }

    const struct ipv4_header* ip = (const struct ipv4_header*)packet;
    
    uint8_t version = (ip->ihl_version >> 4) & 0x0F;
    uint8_t ihl = (ip->ihl_version & 0x0F) * 4;
    
    if (version != 4 || ihl < 20 || len < ihl) {
        return; 
    }

    switch (ip->protocol) {
        case 1:  
            icmp_receive(dev, packet, len); 
            break;
        case 17:  
            udp_receive(dev, packet, len);  
            break;
        default: 
            break;
    }
}

void net_receive(struct net_device* dev, const uint8_t* raw_packet, uint16_t len) {
    if (len < sizeof(struct eth_header)) return;

    struct eth_header* eth = (struct eth_header*)raw_packet;
    uint16_t ethertype = __builtin_bswap16(eth->ethertype);

    if (ethertype == 0x0806) {
        arp_handle_request(dev, raw_packet, len);
        return;
    }

    if (ethertype == 0x0800) { 
        ip_receive(dev, raw_packet + sizeof(struct eth_header), len - sizeof(struct eth_header));
    }
}
