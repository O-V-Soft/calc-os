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

struct icmp_header {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
} __attribute__((packed));

struct arp_header {
    uint16_t hw_type;     
    uint16_t proto_type;
    uint8_t  hw_len;       
    uint8_t  proto_len;  
    uint16_t opcode;      
    uint8_t  sender_mac[6]; 
    uint32_t sender_ip;   
    uint8_t  target_mac[6]; 
    uint32_t target_ip;     
} __attribute__((packed));

void ip_receive(struct net_device* dev, const uint8_t* packet, uint16_t len);
void net_receive(struct net_device* dev, const uint8_t* raw_packet, uint16_t len);

#endif
