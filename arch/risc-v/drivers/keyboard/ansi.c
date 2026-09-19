#include <keyboard.h>
#include <video.h>
#include <stdint.h>
#include <riscv.h>

int ansi_state = 0;

int handle_ansi(uint8_t byte) {
    if (ansi_state == 0 && byte == 0x1B) {
        ansi_state = 1; 
        return 1; 
    }

    if (ansi_state == 1) {
        if (byte == '[') {
            ansi_state = 2; 
            return 1; 
        } else {
            ansi_state = 0; 
            handle_hotkeys(byte);
            return 0;
        }
    }

    if (ansi_state == 2) {
        ansi_state = 0; 
        
        uint8_t code = 0;
        switch (byte) {
            case 'A': code = 0x48; break; 
            case 'B': code = 0x50; break; 
            case 'C': code = 0x4D; break; 
            case 'D': code = 0x4B; break; 
            default: 
                handle_hotkeys(byte);
                return 0;
        }

        handle_hotkeys(code);
        return 1; 
    }

    handle_hotkeys(byte);
    return 0; 
}
