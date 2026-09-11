#include <stdint.h>

#if defined(__riscv)
#include <riscv.h>
#endif

void get_string(char *buffer) {
    int i = 0;

    while (1) {
        char c = uart_get_char(); 
        if (c == 0) continue;

        if (c == '\r' || c == '\n') {
            buffer[i] = '\0';
            return;
        }

        if (c == 0x08 || c == 0x7F) {
            if (i > 0) {
                i--;
                uart_put_char('\b');
                uart_put_char(' ');
                uart_put_char('\b');
            }
            continue;
        }

        if (i < 255) {
            uart_put_char(c); 
            buffer[i] = c;
            i++;
        }
    }
}
