#include <net_dev.h>
#include <net.h>

int icmp_send(struct net_device* dev, uint32_t dest_ip, uint8_t type, uint8_t code, const uint8_t* data, uint16_t data_len) {
    uint8_t icmp_buffer[256];
    if (sizeof(struct icmp_header) + data_len > sizeof(icmp_buffer)) return -1;

    struct icmp_header* icmp = (struct icmp_header*)icmp_buffer;
    uint8_t* payload = icmp_buffer + sizeof(struct icmp_header);

    icmp->type = type; 
    icmp->code = code; 
    icmp->checksum = 0;
    icmp->identifier = __builtin_bswap16(0x0001);
    icmp->sequence = __builtin_bswap16(1);

    for (int i = 0; i < data_len; i++) {
        payload[i] = data[i];
    }

    uint16_t icmp_total_len = sizeof(struct icmp_header) + data_len;
    icmp->checksum = calculate_checksum(icmp, icmp_total_len);

    return ip_send(dev, dest_ip, 1, icmp_buffer, icmp_total_len);
}
