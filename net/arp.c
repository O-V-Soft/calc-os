#include <net_dev.h>
#include <net.h>

void arp_handle_request(struct net_device* dev, const uint8_t* packet, uint16_t len) {
    if (len < sizeof(struct eth_header) + sizeof(struct arp_header)) return;

    struct eth_header* eth = (struct eth_header*)packet;
    struct arp_header* arp = (struct arp_header*)(packet + sizeof(struct eth_header));

    uint32_t our_ip = 0xC0A83202; 
    if (arp->target_ip != our_ip) return;
    if (__builtin_bswap16(arp->opcode) != 1) return; 

    arp->opcode = __builtin_bswap16(2); 

    for (int i = 0; i < 6; i++) {
        arp->target_mac[i] = arp->sender_mac[i];
        arp->sender_mac[i] = dev->mac_addr[i];
    }

    uint32_t temp_ip = arp->sender_ip;
    arp->sender_ip = arp->target_ip;
    arp->target_ip = temp_ip;

    for (int i = 0; i < 6; i++) {
        eth->dest_mac[i] = eth->src_mac[i];
        eth->src_mac[i] = dev->mac_addr[i];
    }

    dev->send_packet(packet, len);
}