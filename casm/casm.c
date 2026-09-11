#include <stdint.h>
#include <fat.h>
#include <idt.h>
#include <utils.h>
#include <vfs.h>

uint8_t asm_buffer[512];
int asm_ptr = 0;

void emit_asm(uint8_t byte) {
    if (asm_ptr >= sizeof(asm_buffer)) return;
    asm_buffer[asm_ptr++] = byte;
}

void assemble_line(const char* line) {
    char* mov_eax_ptr = strstr(line, "mov eax ");
    if (mov_eax_ptr != 0) {
        int value = atoi(mov_eax_ptr + 8); 
        
        emit_asm(0xB8);                
        emit_asm(value & 0xFF);        
        emit_asm((value >> 8) & 0xFF);  
        emit_asm((value >> 16) & 0xFF);
        emit_asm((value >> 24) & 0xFF); 
        return;
    }

    char* mov_al_ptr = strstr(line, "mov al ");
    if (mov_al_ptr != 0) {
        int value = atoi(mov_al_ptr + 7); 
        emit_asm(0xB0);                 
        emit_asm(value & 0xFF);         
        return;
    }

    char* int_ptr = strstr(line, "int ");
    if (int_ptr != 0) {
        int value = atoi(int_ptr + 4);
        emit_asm(0xCD);
        emit_asm(value & 0xFF);
        return;
    }

    if (strstr(line, "cli")) {
        emit_asm(0xFA);
        return;
    }

    if (strstr(line, "sti")) {
        emit_asm(0xFB);
        return;
    }

    if (strstr(line, "hlt")) {
        emit_asm(0xF4);
        return;
    }

    if (strstr(line, "pusha") || strstr(line, "pushad")) {
        emit_asm(0x60);
        return;
    }

    if (strstr(line, "popa") || strstr(line, "popad")) {
        emit_asm(0x61);
        return;
    }

    if (strstr(line, "ret")) {
        emit_asm(0xC3);
        return;
    }

    if (strstr(line, "nop")) {
        emit_asm(0x90);
        return;
    }
}

void assemble_to_bin(const char* out_filename, const char* source_code) {
    for (int i = 0; i < 512; i++) {
        asm_buffer[i] = 0x90; 
    }
    
    asm_ptr = 0;
    char line_buf[64];
    int l_ptr = 0;

    for (int x = 0; x < 64; x++) {
        line_buf[x] = 0;
    }

    for (int i = 0; source_code[i] != '\0'; i++) {
        if (source_code[i] == '\n' || source_code[i] == ';') {
            if (l_ptr > 0) {
                line_buf[l_ptr] = '\0';
                assemble_line(line_buf);
            }
            for (int x = 0; x < 64; x++) {
                line_buf[x] = 0;
            }
            l_ptr = 0; 
        } 
        else if (source_code[i] != '\r') {
            if (l_ptr < 63) {
                line_buf[l_ptr++] = source_code[i];
            }
        }
    }
    
    if (l_ptr > 0) {
        line_buf[l_ptr] = '\0';
        assemble_line(line_buf);
    }
    
    vfs_create((char*)out_filename, asm_buffer, 512);
}
