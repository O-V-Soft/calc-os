[bits 16]
[org 0x7c00]
jmp start_code
nop
OEM_ID db "MSDOS5.0"
bytes_per_sector dw 512
sectors_per_cluster db 1
reserved_sectors dw 1
num_fats db 2
root_entries dw 224
total_sectors_short dw 2880
media_type db 0xF0
fat_size_sectors dw 9
sectors_per_track dw 18
num_heads dw 2
hidden_sectors dd 0
total_sectors_long dd 0
boot_drive_number db 0
reserved1 db 0
boot_signature db 0x29
volume_id dd 0x12345678
volume_label db "NO NAME    "
file_system_type db "FAT12   "
MODE_INFO_BUF equ 0x7E00

start_code:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    mov [boot_drive], dl

    in al, 0x92
    or al, 2
    out 0x92, al

    mov cx, 0x0100          

setup_vbe_loop:
    cmp cx, 0x0200          
    je error_handler

    push cx                 

    mov ax, 0x4F01
    mov di, MODE_INFO_BUF   
    int 0x10
    
    pop cx                  
    
    cmp ax, 0x004F          
    jne next_mode

    mov bx, MODE_INFO_BUF
    cmp word [bx + 18], 1024
    jne next_mode
    cmp word [bx + 20], 768   
    jne next_mode
    cmp byte [bx + 25], 8      
    jne next_mode

    mov eax, [bx + 40]
    mov [0x0500], eax

    mov bx, cx
    or bx, 0x4000           
    mov ax, 0x4F02
    int 0x10
    cmp ax, 0x004F
    jne error_handler
    jmp load_disk

next_mode:
    inc cx                  
    jmp setup_vbe_loop

load_disk:
    mov cx, 6           
    mov dword [current_lba], 0  
    mov word [target_seg], 0x1000

read_loop:
    push cx
    
    mov eax, [current_lba]
    mov [disk_packet + 8], eax 
    
    mov ax, [target_seg]
    mov [disk_packet + 6], ax 
    
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, disk_packet
    int 0x13
    jc error_handler

    add dword [current_lba], 32
    
    add word [target_seg], 0x0400
    
    pop cx
    loop read_loop

    cli
    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:init_32bit

error_handler:
    cli
    hlt
    jmp error_handler

[bits 32]
init_32bit:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x90000
    mov ebp, esp

    jmp 0x14200

boot_drive db 0
target_seg dw 0x1000
current_lba dd 0       

align 4
disk_packet:
    db 0x10      
    db 0             
    dw 32
    dw 0x0000        
    dw 0x1000       
    dq 0            

gdt_start:
    dq 0                

gdt_code:
    dw 0xFFFF          
    dw 0x0000          
    db 0x00            
    db 10011010b      
    db 11001111b 
    db 0x00      

gdt_data:
    dw 0xFFFF         
    dw 0x0000          
    db 0x00             
    db 10010010b       
    db 11001111b      
    db 0x00             
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55
