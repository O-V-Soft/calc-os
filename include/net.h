#ifndef NET_H
#define NET_H

#include <stdint.h>

struct eth_header {
    uint8_t  dest_mac[6];
    uint8_t  src_mac[6];
    uint16_t ethertype;
} __attribute__((packed));

struct ipv4_header {
    uint8_t  ihl_version; 
    uint8_t  tos;       
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_fragment_offset;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
} __attribute__((packed));

struct udp_header {
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum; 
} __attribute__((packed));

int net_send(struct net_device* dev, uint32_t dest_ip, uint16_t src_port, uint16_t dest_port, const uint8_t* payload, uint16_t payload_len);
void net_receive(struct net_device* dev, const uint8_t* raw_packet, uint16_t len);

#endif