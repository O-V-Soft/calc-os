#include <stdint.h>
#include <riscv.h>

volatile uint8_t* uart_reg_base = (volatile uint8_t*)0x10000000;

void uart_put_char(char c) {
    while ((*UART_REG(5) & 0x20) == 0);
    *UART_REG(0) = c;
}

void uart_printk(const char* str) {
    while (*str) {
        if (*str == '\n') uart_put_char('\r');
        uart_put_char(*str++);
    }
}

char uart_get_char() {
    if ((*UART_LSR & UART_LSR_DR) == 0) {
        return 0; 
    }
    
    return (char)(*UART_RBR);
}
