#include <net_dev.h>
#include <net.h>
#include <utils.h>

uint8_t target_host_mac[6];
int host_mac_resolved = 0;

void arp_receive(struct net_device* dev, const uint8_t* packet, uint16_t len) {
    if (len < sizeof(struct arp_header)) return;

    const struct arp_header* arp = (const struct arp_header*)packet;
    uint16_t opcode = __builtin_bswap16(arp->opcode);

    if (opcode == 1) {
        if (arp->target_ip != OUR_IP) return;

        uint8_t reply_buf[60];
        __builtin_memset(reply_buf, 0, sizeof(reply_buf));

        struct eth_header* eth = (struct eth_header*)reply_buf;
        struct arp_header* rep_arp = (struct arp_header*)(reply_buf + sizeof(struct eth_header));

        for (int i = 0; i < 6; i++) {
            eth->dest_mac[i] = arp->sender_mac[i];
            eth->src_mac[i] = dev->mac_addr[i]; 
        }
        eth->ethertype = __builtin_bswap16(0x0806);

        rep_arp->hw_type = __builtin_bswap16(1);
        rep_arp->proto_type = __builtin_bswap16(0x0800);
        rep_arp->hw_len = 6;
        rep_arp->proto_len = 4;
        rep_arp->opcode = __builtin_bswap16(2); 

        for (int i = 0; i < 6; i++) {
            rep_arp->sender_mac[i] = dev->mac_addr[i]; 
            rep_arp->target_mac[i] = arp->sender_mac[i];
        }
        rep_arp->sender_ip = OUR_IP;
        rep_arp->target_ip = arp->sender_ip;

        dev->send_packet(reply_buf, 60);
    } 
    else if (opcode == 2) {
        for (int i = 0; i < 6; i++) {
            target_host_mac[i] = arp->sender_mac[i];
        }
        host_mac_resolved = 1;
    }
}

void arp_request(struct net_device* dev, uint32_t target_ip) {
    uint8_t request_buf[60];
    memset(request_buf, 0, sizeof(request_buf));

    struct eth_header* eth = (struct eth_header*)request_buf;
    struct arp_header* arp = (struct arp_header*)(request_buf + sizeof(struct eth_header));

    for (int i = 0; i < 6; i++) {
        eth->dest_mac[i] = 0xFF; 
        eth->src_mac[i] = dev->mac_addr[i]; 
    }
    eth->ethertype = __builtin_bswap16(0x0806); 

    arp->hw_type = __builtin_bswap16(1);        
    arp->proto_type = __builtin_bswap16(0x0800);
    arp->hw_len = 6;
    arp->proto_len = 4;
    arp->opcode = __builtin_bswap16(1);        

    for (int i = 0; i < 6; i++) {
        arp->sender_mac[i] = dev->mac_addr[i];   
        arp->target_mac[i] = 0;                  
    }
    arp->sender_ip = OUR_IP;
    arp->target_ip = target_ip;

    dev->send_packet(request_buf, sizeof(request_buf));
}
