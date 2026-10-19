#include <video.h>
#include <utils.h>
#include <idt.h>
#include <stdint.h>
#include <pci.h>
#include <net_dev.h>
#include <net.h>

static volatile uint8_t* rtl_mmio;
static uint8_t rtl_rx_buffer[8192 + 16 + 1500] __attribute__((aligned(16)));
static volatile uint8_t rtl_tx_buffer[4][2048] __attribute__((aligned(4)));
static uint8_t tx_counter = 0;
static uint16_t rx_offset = 0;
static uint8_t mac_addr[6];

uint8_t rtl_irq;

void irq_unmask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = 0x21; 
    } else {
        port = 0xA1; 
        irq -= 8;
    }

    value = inb(port);
    value &= ~(1 << irq); 
    outb(port, value);
}

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

    rtl_irq = pci_read_config_byte(devices[index].bus, devices[index].slot, devices[index].func, 0x3C);
        
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

    *(volatile uint32_t*)(rtl_mmio + 0x44) = (1 << 7) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0);

    *(volatile uint32_t*)(rtl_mmio + 0x30) = rx_phys;

    *(volatile uint16_t*)(rtl_mmio + 0x38) = 8192 - 16; 
    
    *(volatile uint16_t*)(rtl_mmio + 0x3C) = 0x0005;

    rtl_mmio[0x37] = 0x0C;

    tx_counter = 0;
    rx_offset = 0; 

    irq_unmask(rtl_irq);
    set_idt_gate(0x20 + rtl_irq, (uint32_t)rtl8139_wrapper, 0x08, 0x8E);
}

int rtl8139_send(const uint8_t* data, uint16_t len) {
    if (len > 1792) return -1; 
    
    uint32_t phys_tx_buf = (uint32_t)rtl_tx_buffer[tx_counter];
    
    for (int i = 0; i < len; i++) {
        rtl_tx_buffer[tx_counter][i] = data[i];
    }

    uint32_t frame_len = len < 60 ? 60 : len;
    uint8_t tx_reg_offset = tx_counter * 4;
    
    *(volatile uint32_t*)((uint32_t)rtl_mmio + 0x20 + tx_reg_offset) = phys_tx_buf;
    *(volatile uint32_t*)((uint32_t)rtl_mmio + 0x10 + tx_reg_offset) = frame_len;

    tx_counter = (tx_counter + 1) % 4;
    return 0;
}

int rtl8139_poll(uint8_t* buffer, uint16_t max_len) {
    uint8_t* header = rtl_rx_buffer + rx_offset;
    uint16_t status = header[0] | (header[1] << 8);
    if (!(status & 0x01)) return 0;

    uint16_t length = header[2] | (header[3] << 8);
    uint16_t packet_len = length - 4;

    if (packet_len > max_len) {
        packet_len = max_len;
    }

    uint8_t* payload = header + 4;
    for (int i = 0; i < packet_len; i++) {
        buffer[i] = payload[i];
    }

    rx_offset = (rx_offset + length + 4 + 3) & ~3;
    rx_offset = rx_offset % 8192;
    
    uint16_t capr_val = (rx_offset == 0) ? (8192 - 16) : (rx_offset - 16);
    *(volatile uint16_t*)(rtl_mmio + 0x38) = capr_val;

    return packet_len;
}

void rtl8139_handler() {
    if (!rtl_mmio) return;

    uint16_t status = *(volatile uint16_t*)(rtl_mmio + 0x3E);

    *(volatile uint16_t*)(rtl_mmio + 0x3E) = status;

    if (status & 0x0001) {
        uint8_t rx_buf[1500];
        int len = rtl8139_poll(rx_buf, sizeof(rx_buf)); 
        if (len > 0) {
            net_receive(&rtl_device, rx_buf, len);
        }
    }
}

struct net_device rtl_device = {
    .name = "rtl8139",
    .init = rtl8139_init,
    .send_packet = rtl8139_send,
    .poll_packet = rtl8139_poll,
};

void rtl8139_register() {
    for(int i = 0; i < 6; i++) rtl_device.mac_addr[i] = mac_addr[i];
    register_net_device(&rtl_device);
}
