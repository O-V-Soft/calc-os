#include <stdint.h>
#include <mm.h>

static Block* freeList = NULL;

void init_kernel_heap(void* start_addr, size_t heap_size) {
    uintptr_t raw_addr = (uintptr_t)start_addr;
    uintptr_t adjustment = (ALIGNMENT - (raw_addr & (ALIGNMENT - 1))) & (ALIGNMENT - 1);
    freeList = NULL;

    if (!start_addr || raw_addr > (uintptr_t)-1 - adjustment ||
        heap_size <= adjustment || heap_size - adjustment <= BLOCK_SIZE) {
        return;
    }

    uintptr_t aligned_addr = raw_addr + adjustment;
    size_t usable_size = heap_size - (size_t)adjustment;
    size_t block_size = (usable_size - BLOCK_SIZE) & ~(ALIGNMENT - 1);
    if (block_size == 0) {
        return;
    }

    freeList = (Block*)aligned_addr;
    freeList->size = block_size;
    freeList->free = 1;
    freeList->next = NULL;
}

void init_memory_manager() {
    init_kernel_heap((void*)HEAP_START, HEAP_SIZE);
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    if (size > (size_t)-1 - (ALIGNMENT - 1)) return NULL;

    size = ALIGN(size);

    Block* current = freeList;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size - size >= BLOCK_SIZE + ALIGNMENT) {
                Block* new_block = (Block*)((uint8_t*)current + BLOCK_SIZE + size);
                new_block->size = current->size - size - BLOCK_SIZE;
                new_block->free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->free = 0;
            return (void*)((uint8_t*)current + BLOCK_SIZE);
        }
        current = current->next;
    }
    return NULL;
}

void kfree(void* ptr) {
    if (!ptr) return;

    Block* current = freeList;
    while (current) {
        if ((uint8_t*)current + BLOCK_SIZE == (uint8_t*)ptr) {
            if (current->free) return;
            current->free = 1;
            break;
        }
        current = current->next;
    }

    if (!current) return;

    current = freeList;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += BLOCK_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}
