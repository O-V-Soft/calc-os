#include <stdint.h>
#include <net_dev.h>
#include <net.h>
#include <video.h>
#include <utils.h>

uint32_t ipv4_parse(const char *str) {
    uint32_t addr = 0;
    uint32_t current_val = 0;

    while (*str != '\0') {
        if (*str >= '0' && *str <= '9') {
            current_val = current_val * 10 + (*str - '0');
        } else if (*str == '.') {
            addr = (addr << 8) | (current_val & 0xFF);
            current_val = 0;
        }
        str++;
    }
    addr = (addr << 8) | (current_val & 0xFF);

    return __builtin_bswap32(addr); 
}

uint16_t calculate_checksum(const void* data, size_t len) {
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
        case 1: { 
            const struct icmp_header *icmp = (const struct icmp_header *)(packet + ihl);
            if (icmp->type == 0) { 
                print("Ping reply received\n", COLOR_WHITE);
            }
            break;
        }
        default: 
            break;
    }
}

int ip_send(struct net_device* dev, uint32_t dest_ip, uint8_t protocol, const uint8_t* payload, uint16_t payload_len) {
    uint8_t dest_mac[6];

    if (!arp_cache_lookup(dest_ip, dest_mac)) {
        arp_request(dev, dest_ip);
        return -1; 
    }

    uint16_t ip_total_len = sizeof(struct ipv4_header) + payload_len;
    uint16_t total_len = sizeof(struct eth_header) + ip_total_len;

    uint8_t packet[128];
    if (total_len > sizeof(packet)) return -2; 
    __builtin_memset(packet, 0, total_len < 60 ? 60 : total_len);

    struct eth_header  *eth = (struct eth_header*)packet;
    struct ipv4_header *ip  = (struct ipv4_header*)(packet + sizeof(struct eth_header));
    uint8_t *dest_payload   = packet + sizeof(struct eth_header) + sizeof(struct ipv4_header);

    for (int i = 0; i < 6; i++) {
        eth->dest_mac[i] = dest_mac[i];
        eth->src_mac[i] = dev->mac_addr[i];
    }
    eth->ethertype = __builtin_bswap16(0x0800);

    ip->ihl_version = (4 << 4) | 5;
    ip->tos = 0;
    ip->total_length = __builtin_bswap16(ip_total_len);
    ip->id = __builtin_bswap16(0x1234);
    ip->flags_fragment_offset = 0;
    ip->ttl = 64;
    ip->protocol = protocol; 
    ip->checksum = 0;
    ip->src_ip = OUR_IP;
    ip->dest_ip = dest_ip;
    ip->checksum = calculate_checksum(ip, sizeof(struct ipv4_header));

    for (int i = 0; i < payload_len; i++) {
        dest_payload[i] = payload[i];
    }

    return dev->send_packet(packet, total_len < 60 ? 60 : total_len);
}
