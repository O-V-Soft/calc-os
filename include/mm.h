#ifndef MM_H
#define MM_H
#include <stdint.h>

#define BLOCK_SIZE sizeof(Block)
#define ALIGNMENT sizeof(void*)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#if defined(__riscv)
    #define HEAP_START 0x80500000          
    #define HEAP_SIZE  (4 * 1024 * 1024)  
#else
    #define HEAP_START 0x200000          
    #define HEAP_SIZE  (4 * 1024 * 1024)  
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef struct Block {
    size_t size;
    int free;
    struct Block* next;
} Block;

void init_memory_manager();
void* kmalloc(size_t size);
void kfree(void* ptr);

extern __attribute__((section(".bss.safe"), aligned(4096))) uint32_t page_directory[1024];

void init_paging();
void enable_paging();

#endif
