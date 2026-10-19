#include <net_dev.h>
#include <net.h>

struct arp_cache_entry arp_cache[ARP_CACHE_SIZE];

int arp_cache_lookup(uint32_t ip, uint8_t *mac_out) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].resolved && arp_cache[i].ip == ip) {
            for (int j = 0; j < 6; j++) {
                mac_out[j] = arp_cache[i].mac[j];
            }
            return 1; 
        }
    }
    return 0;
}

void arp_cache_update(uint32_t ip, const uint8_t *mac) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].ip == ip || arp_cache[i].resolved == 0) {
            arp_cache[i].ip = ip;
            for (int j = 0; j < 6; j++) {
                arp_cache[i].mac[j] = mac[j];
            }
            arp_cache[i].resolved = 1;
            return;
        }
    }
}
