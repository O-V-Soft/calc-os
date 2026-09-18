#include <stdint.h>
#include <riscv.h>
#include <pci.h>
#include <virtio_gpu.h>
#include <video.h>

#define VIRTIO_BASE 0x10008000
#define VIRTIO_REG(off) (*(volatile uint32_t *)(VIRTIO_BASE + (off)))

#define VIRTIO_MMIO_MAGIC            VIRTIO_REG(0x000)
#define VIRTIO_MMIO_VERSION          VIRTIO_REG(0x004)
#define VIRTIO_MMIO_GUEST_PAGE_SIZE  VIRTIO_REG(0x028)
#define VIRTIO_MMIO_QUEUE_SEL        VIRTIO_REG(0x030)
#define VIRTIO_MMIO_QUEUE_NUM        VIRTIO_REG(0x038)
#define VIRTIO_MMIO_QUEUE_ALIGN      VIRTIO_REG(0x03c)
#define VIRTIO_MMIO_QUEUE_PFN        VIRTIO_REG(0x040)
#define VIRTIO_MMIO_QUEUE_NOTIFY     VIRTIO_REG(0x050)
#define VIRTIO_MMIO_STATUS           VIRTIO_REG(0x070)

static uint8_t queue_mem[8192] __attribute__((aligned(4096)));
uint32_t frame_buffer[SCREEN_WIDTH * SCREEN_HEIGHT] __attribute__((aligned(4096)));
static uint8_t gpu_resp_buf[512] __attribute__((aligned(16)));

static struct virtq_desc *desc_ring;
static struct virtq_avail *avail_ring;
static struct virtq_used *used_ring;

static uint16_t free_head = 0;
static uint16_t last_used_idx = 0;

static void exec_cmd(void *cmd, uint32_t cmd_len) {
    uint16_t head = free_head;

    desc_ring[head].addr = (uint64_t)(uintptr_t)cmd;
    desc_ring[head].len = cmd_len;
    desc_ring[head].flags = 1;
    desc_ring[head].next = head + 1;

    desc_ring[head + 1].addr = (uint64_t)(uintptr_t)gpu_resp_buf;
    desc_ring[head + 1].len = sizeof(gpu_resp_buf);
    desc_ring[head + 1].flags = 2;
    desc_ring[head + 1].next = 0;

    uint16_t avail_idx = avail_ring->idx;
    avail_ring->ring[avail_idx % QUEUE_SIZE] = head;

    __asm__ volatile("fence rw, rw" ::: "memory");
    avail_ring->idx = avail_idx + 1;
    __asm__ volatile("fence rw, rw" ::: "memory");

    VIRTIO_MMIO_QUEUE_NOTIFY = 0;

    while (used_ring->idx == last_used_idx) {
        __asm__ volatile("nop");
    }

    last_used_idx = used_ring->idx;
    free_head = (free_head + 2) % QUEUE_SIZE;
}

void init_gpu() {
    desc_ring = (struct virtq_desc *)queue_mem;
    avail_ring = (struct virtq_avail *)(queue_mem + QUEUE_SIZE * sizeof(struct virtq_desc));
    used_ring = (struct virtq_used *)(queue_mem + 4096);

    VIRTIO_MMIO_STATUS = 0;
    VIRTIO_MMIO_STATUS = VIRTIO_MMIO_STATUS | 1;
    VIRTIO_MMIO_STATUS = VIRTIO_MMIO_STATUS | 2;

    VIRTIO_MMIO_GUEST_PAGE_SIZE = 4096;

    VIRTIO_MMIO_QUEUE_SEL = 0;
    VIRTIO_MMIO_QUEUE_NUM = QUEUE_SIZE;
    VIRTIO_MMIO_QUEUE_ALIGN = 4096;
    VIRTIO_MMIO_QUEUE_PFN = (uint32_t)(((uintptr_t)queue_mem) >> 12);

    VIRTIO_MMIO_STATUS = VIRTIO_MMIO_STATUS | 4;

    struct virtio_gpu_resource_create_2d create = {
        .hdr = { .type = 0x0101 },
        .resource_id = 1,
        .format = 1,
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT
    };
    exec_cmd(&create, sizeof(create));

    struct virtio_gpu_resource_attach_backing attach = {
        .hdr = { .type = 0x0106 },
        .resource_id = 1,
        .nr_entries = 1,
        .entries = {
            {
                .addr = (uint64_t)(uintptr_t)frame_buffer,
                .length = SCREEN_WIDTH * SCREEN_HEIGHT * 4,
                .padding = 0
            }
        }
    };
    exec_cmd(&attach, sizeof(attach));

    struct virtio_gpu_set_scanout scanout = {
        .hdr = { .type = 0x0103 },
        .r = { .x = 0, .y = 0, .width = SCREEN_WIDTH, .height = SCREEN_HEIGHT },
        .scanout_id = 0,
        .resource_id = 1
    };
    exec_cmd(&scanout, sizeof(scanout));
}

void flush_gpu() {
    struct virtio_gpu_transfer_to_host_2d xfer = {
        .hdr = { .type = 0x0105 },
        .r = { .x = 0, .y = 0, .width = SCREEN_WIDTH, .height = SCREEN_HEIGHT },
        .offset = 0,
        .resource_id = 1,
        .padding = 0
    };
    exec_cmd(&xfer, sizeof(xfer));

    struct virtio_gpu_resource_flush flush = {
        .hdr = { .type = 0x0104 },
        .r = { .x = 0, .y = 0, .width = SCREEN_WIDTH, .height = SCREEN_HEIGHT },
        .resource_id = 1,
        .padding = 0
    };
    exec_cmd(&flush, sizeof(flush));
}
