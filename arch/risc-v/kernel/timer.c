#include <stdint.h>
#include <riscv.h>

unsigned int timer_ticks = 0; 

void init_timer() {
    __asm__ __volatile__("csrw mtvec, %0" : : "r"(trap_vector));

    uint64_t current_time = *RISCV_CLINT_MTIME;
    *RISCV_CLINT_MTIMECMP = current_time + TIMER_FREQUENCY_HZ;

    __asm__ __volatile__("csrs mie, %0" : : "r"(1 << 7));
    
    __asm__ __volatile__("csrs mstatus, %0" : : "r"(1 << 3));
}

void timer_handler() {
    unsigned long mcause;
    __asm__ __volatile__("csrr %0, mcause" : "=r"(mcause));

    if (mcause == 0x80000007) {
        timer_ticks++; 

        *RISCV_CLINT_MTIMECMP = *RISCV_CLINT_MTIME + TIMER_FREQUENCY_HZ;
    } else {
        while(1);
    }
}