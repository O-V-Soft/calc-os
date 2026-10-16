#include <stdint.h>
#include <net_dev.h>
#include <net.h>
#include <video.h>

int udp_send(struct net_device* dev, uint32_t dest_ip, uint16_t src_port, uint16_t dest_port, const uint8_t* payload, uint16_t payload_len) {
    if (!dev) return -1;

    uint8_t frame[1500];
    
    uint16_t udp_len = sizeof(struct udp_header) + payload_len;
    uint16_t ip_len  = sizeof(struct ipv4_header) + udp_len;
    uint16_t total_frame_len = sizeof(struct eth_header) + ip_len;

    if (total_frame_len > sizeof(frame)) return -2; 

    struct eth_header* eth = (struct eth_header*)frame;
    for(int i = 0; i < 6; i++) {
        eth->src_mac[i] = dev->mac_addr[i];
        eth->dest_mac[i] = 0xFF; 
    }
    eth->ethertype = __builtin_bswap16(0x0800); 

    struct ipv4_header* ip = (struct ipv4_header*)(frame + sizeof(struct eth_header));
    ip->ihl_version = 0x45; 
    ip->tos = 0x00;
    ip->total_length = __builtin_bswap16(ip_len);
    ip->id = 0x0000;
    ip->flags_fragment_offset = 0x0000;
    ip->ttl = 64;
    ip->protocol = 17; 
    ip->checksum = 0; 
    ip->src_ip = __builtin_bswap32(0xC0A80102);
    ip->dest_ip = __builtin_bswap32(dest_ip);

    struct udp_header* udp = (struct udp_header*)(frame + sizeof(struct eth_header) + sizeof(struct ipv4_header));
    udp->src_port = __builtin_bswap16(src_port);
    udp->dest_port = __builtin_bswap16(dest_port);
    udp->length = __builtin_bswap16(udp_len);
    udp->checksum = 0x0000; 

    uint8_t* data_dest = frame + sizeof(struct eth_header) + sizeof(struct ipv4_header) + sizeof(struct udp_header);
    for (uint16_t i = 0; i < payload_len; i++) {
        data_dest[i] = payload[i];
    }

    return dev->send_packet(frame, total_frame_len);
}

void udp_receive(struct net_device* dev, const uint8_t* raw_packet, uint16_t len) {
    if (len < sizeof(struct eth_header)) return;

    struct eth_header* eth = (struct eth_header*)raw_packet;
    
    if (__builtin_bswap16(eth->ethertype) != 0x0800) return;

    uint16_t ip_offset = sizeof(struct eth_header);
    if (len < ip_offset + sizeof(struct ipv4_header)) return;

    struct ipv4_header* ip = (struct ipv4_header*)(raw_packet + ip_offset);
    
    if (ip->protocol != 17) return;

    uint8_t ihl = (ip->ihl_version & 0x0F) * 4;
    uint16_t udp_offset = ip_offset + ihl;
    
    if (len < udp_offset + sizeof(struct udp_header)) return;

    struct udp_header* udp = (struct udp_header*)(raw_packet + udp_offset);
    uint16_t dest_port = __builtin_bswap16(udp->dest_port);

    if (dest_port != 1024) return;

    uint16_t udp_len = __builtin_bswap16(udp->length);
    
    const uint8_t* udp_payload = raw_packet + udp_offset + sizeof(struct udp_header);
    uint16_t payload_size = udp_len >= sizeof(struct udp_header) ? (udp_len - sizeof(struct udp_header)) : 0;

    for (uint16_t i = 0; i < payload_size; i++) {
        put_char(udp_payload[i], COLOR_WHITE);
    }
}
