#include <net_dev.h>
#include <net.h>

static uint16_t calculate_checksum(const void* data, size_t len) {
    const uint16_t* ptr = (const uint16_t*)data;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    if (len > 0) {
        sum += *(const uint8_t*)ptr;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~((uint16_t)sum);
}

void icmp_handle_echo(struct net_device* dev, const uint8_t* packet, uint16_t len) {
    if (len < sizeof(struct eth_header) + sizeof(struct ipv4_header) + sizeof(struct icmp_header)) {
        return; 
    }

    struct eth_header* eth = (struct eth_header*)packet;
    uint16_t ip_offset = sizeof(struct eth_header);
    struct ipv4_header* ip = (struct ipv4_header*)(packet + ip_offset);
    
    uint8_t ihl = (ip->ihl_version & 0x0F) * 4;
    uint16_t icmp_offset = ip_offset + ihl;
    struct icmp_header* icmp = (struct icmp_header*)(packet + icmp_offset);

    if (icmp->type != 8) {
        return; 
    }

    for (int i = 0; i < 6; i++) {
        uint8_t temp = eth->src_mac[i];
        eth->src_mac[i] = eth->dest_mac[i];
        eth->dest_mac[i] = temp;
    }

    uint32_t temp_ip = ip->src_ip;
    ip->src_ip = ip->dest_ip;
    ip->dest_ip = temp_ip;
    
    ip->ttl = 64;
    ip->checksum = 0;
    ip->checksum = calculate_checksum(ip, ihl); 

    icmp->type = 0;
    icmp->code = 0;
    icmp->checksum = 0;

    uint16_t icmp_len = len - icmp_offset;
    icmp->checksum = calculate_checksum(icmp, icmp_len);

    dev->send_packet(packet, len);
}

void icmp_receive(struct net_device* dev, const uint8_t* packet, uint16_t len) {
    if (len < sizeof(struct ipv4_header)) return;

    const struct ipv4_header* ip = (const struct ipv4_header*)packet;
    uint8_t ihl = (ip->ihl_version & 0x0F) * 4;

    if (len < ihl + sizeof(struct icmp_header)) return;

    const struct icmp_header* icmp = (const struct icmp_header*)(packet + ihl);

    if (icmp->type == 8) {
        icmp_handle_echo(dev, packet, len);
    }
}
