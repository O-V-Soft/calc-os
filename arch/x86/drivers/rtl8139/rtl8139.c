#include <video.h>
#include <utils.h>
#include <idt.h>
#include <stdint.h>
#include <pci.h>
#include <video.h>

volatile uint8_t* rtl_mmio;

#define RX_BUFFER_SIZE 0x8000 

uint8_t rtl_rx_buffer[RX_BUFFER_SIZE + 2048] __attribute__((aligned(16)));
volatile uint8_t rtl_tx_buffer[4][2048] __attribute__((aligned(4)));

static uint8_t tx_counter = 0;
static uint8_t mac_addr[6]; 
static uint16_t rx_offset = 0; 

int rtl8139_find() {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].vendor_id == 0x10EC && devices[i].device_id == 0x8139) {
            return i;
        }
    }
    return -1;
}

uint32_t rtl8139_get_mmio() {
    uint32_t mmio_base;

    int index = rtl8139_find();
    if (index < 0) return 0;

    mmio_base = pci_read_config_dword(devices[index].bus, devices[index].slot, devices[index].func, 0x14);
    mmio_base = mmio_base & ~0xF;

    return mmio_base;
}

void rtl8139_init() {
    uint32_t rx_phys = (uint32_t)rtl_rx_buffer;

    int index = rtl8139_find();
    if (index < 0) return;

    uint16_t cmd = pci_read_config_word(devices[index].bus, devices[index].slot, devices[index].func, 0x04);
    cmd = cmd | 0x0007; 
    pci_write_config_word(devices[index].bus, devices[index].slot, devices[index].func, 0x04, cmd);

    uint32_t mmio_base = rtl8139_get_mmio();
    if (mmio_base == 0) return;
    rtl_mmio = (volatile uint8_t*)mmio_base;

    for (int i = 0; i < 6; i++) {
        mac_addr[i] = rtl_mmio[i];
    }

    rtl_mmio[0x37] = 0x10;
    uint32_t timeout = 500000;
    while ((rtl_mmio[0x37] & 0x10) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) return;

    *(volatile uint32_t*)(rtl_mmio + 0x44) = 0x0000E70F;

    *(volatile uint32_t*)(rtl_mmio + 0x30) = rx_phys;

    *(volatile uint16_t*)(rtl_mmio + 0x38) = 0;
    *(volatile uint16_t*)(rtl_mmio + 0x3A) = 0;

    *(volatile uint16_t*)(rtl_mmio + 0x3C) = 0x0005;

    rtl_mmio[0x37] = 0x0C;

    tx_counter = 0;
    rx_offset = 0; 
}

uint8_t read_pack() {
    int is_rtl8139_found = rtl8139_find();
    if (!is_rtl8139_found) return 0;

    if (rtl_mmio[0x37] & 0x01) {
        return 0;
    }
    
    uint8_t* header = rtl_rx_buffer + rx_offset;
    uint16_t status = header[0] | (header[1] << 8);
    if (!(status & 0x01)) {
        return 0;
    }

    uint16_t length = header[2] | (header[3] << 8);
    uint8_t* payload = header + 4;
    uint16_t ethertype = (payload[12] << 8) | payload[13];
    
    if (ethertype == 0x0800) {
        uint8_t protocol = payload[23];
        if (protocol == 17) {
            uint16_t dst_port = (payload[14 + 20 + 2] << 8) | payload[14 + 20 + 3];
            if (dst_port == 51131) {
                uint16_t udp_length = (payload[14 + 20 + 4] << 8) | payload[14 + 20 + 5];
                uint16_t udp_payload_size = udp_length - 8;
                uint8_t* udp_payload = payload + 42;

                if (udp_payload_size + 42 > length) {
                    udp_payload_size = length - 42;
                }

                for (int i = 0; i < udp_payload_size; i++) {
                    put_char(udp_payload[i], 15);
                }
            }
        }
    }

    rx_offset = (rx_offset + length + 4 + 3) & ~3;
    rx_offset = rx_offset % RX_BUFFER_SIZE;
    *(volatile uint16_t*)(rtl_mmio + 0x3A) = rx_offset - 16;

    return 1;
}

void send_pack(uint8_t* payload, uint16_t payload_size, uint8_t dest_mac[6]) {
    int is_rtl8139_found = rtl8139_find();
    if (!is_rtl8139_found) return;

    uint32_t phys_tx_buf = (uint32_t)rtl_tx_buffer[tx_counter];

    for (int i = 0; i < 6; i++) {
        rtl_tx_buffer[tx_counter][i] = dest_mac[i];
        rtl_tx_buffer[tx_counter][i + 6] = mac_addr[i];
    }

    rtl_tx_buffer[tx_counter][12] = 0x08;
    rtl_tx_buffer[tx_counter][13] = 0x00;
    rtl_tx_buffer[tx_counter][14] = 0x45;
    rtl_tx_buffer[tx_counter][15] = 0x00;

    uint16_t udp_length = 8 + payload_size;
    uint16_t ip_total = 20 + udp_length;

    rtl_tx_buffer[tx_counter][16] = (ip_total >> 8) & 0xFF;
    rtl_tx_buffer[tx_counter][17] = ip_total & 0xFF;
    rtl_tx_buffer[tx_counter][18] = 0x00;
    rtl_tx_buffer[tx_counter][19] = 0x00;
    rtl_tx_buffer[tx_counter][20] = 0x00;
    rtl_tx_buffer[tx_counter][21] = 0x00;
    rtl_tx_buffer[tx_counter][22] = 0x40;
    rtl_tx_buffer[tx_counter][23] = 0x11;
    rtl_tx_buffer[tx_counter][24] = 0x00;
    rtl_tx_buffer[tx_counter][25] = 0x00;
    rtl_tx_buffer[tx_counter][26] = 0xC0;
    rtl_tx_buffer[tx_counter][27] = 0xA8;
    rtl_tx_buffer[tx_counter][28] = 0x01;
    rtl_tx_buffer[tx_counter][29] = 0x03;
    rtl_tx_buffer[tx_counter][30] = 0xFF;
    rtl_tx_buffer[tx_counter][31] = 0xFF;
    rtl_tx_buffer[tx_counter][32] = 0xFF;
    rtl_tx_buffer[tx_counter][33] = 0xFF;
    rtl_tx_buffer[tx_counter][34] = 0xC8;
    rtl_tx_buffer[tx_counter][35] = 0x5F;
    rtl_tx_buffer[tx_counter][36] = 0xC7;
    rtl_tx_buffer[tx_counter][37] = 0xBB;
    rtl_tx_buffer[tx_counter][38] = (udp_length >> 8) & 0xFF;
    rtl_tx_buffer[tx_counter][39] = udp_length & 0xFF;
    rtl_tx_buffer[tx_counter][40] = 0x00;
    rtl_tx_buffer[tx_counter][41] = 0x00;

    for (int i = 0; i < payload_size; i++) {
        rtl_tx_buffer[tx_counter][42 + i] = payload[i];
    }

    uint32_t frame_len = 42 + payload_size;
    if (frame_len < 60) frame_len = 60;

    uint8_t tx_reg_offset = tx_counter * 4;
    *(volatile uint32_t*)((uint32_t)rtl_mmio + 0x20 + tx_reg_offset) = phys_tx_buf;
    *(volatile uint32_t*)((uint32_t)rtl_mmio + 0x10 + tx_reg_offset) = frame_len;

    tx_counter = (tx_counter + 1) % 4;
}
