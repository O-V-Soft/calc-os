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

#define OUR_IP ((192) | (168 << 8) | (50 << 16) | (2 << 24))
#define TARGET_IP ((192) | (168 << 8) | (50 << 16) | (1 << 24))

extern uint8_t target_host_mac[6];
extern int host_mac_resolved;

void arp_receive(struct net_device* dev, const uint8_t* packet, uint16_t len);
void arp_request(struct net_device* dev, uint32_t target_ip);

uint16_t calculate_checksum(const void* data, size_t len);
void ip_receive(struct net_device* dev, const uint8_t* packet, uint16_t len);
int ip_send(struct net_device* dev, uint32_t dest_ip, uint8_t protocol, const uint8_t* payload, uint16_t payload_len);

void net_receive(struct net_device* dev, const uint8_t* raw_packet, uint16_t len);
int net_ping(struct net_device* dev);

int icmp_send(struct net_device* dev, uint32_t dest_ip, uint8_t type, uint8_t code, const uint8_t* data, uint16_t data_len);

#endif
