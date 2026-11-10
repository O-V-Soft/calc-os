#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <sound.h>
#include <forth.h>
#include <fat.h>
#include <mm.h>
#include <pci.h>
#include <vfs.h>
#include <net.h>
#include <net_dev.h>

uint32_t syscall_handler(struct registers *regs) {
    uint32_t syscall_num = regs->eax;
    uint32_t arg1 = regs->ebx;
    uint32_t arg2 = regs->ecx;
    uint32_t arg3 = regs->edx;

    uint32_t ret_val = 0;

    switch (syscall_num) {
        case SYS_EXIT:
            bin_running = 0;
            delete_task(current_task->pid);
            ret_val = 0;
            break;

        case SYS_OPEN: {
            const char* path = (const char*)arg1;
            
            ret_val = vfs_open(path);   
            break;
        }
        
        case SYS_WRITE: {
            if (arg1 == 1 || arg1 == 2) {
                const char *buf = (const char*)arg2;
                int count = (int)arg3; 
        
                for (size_t i = 0; i < count; i++) {
                    put_char(buf[i], COLOR_WHITE); 
                }
        
                ret_val = count;
            } else {
                ret_val = -EBADF; 
            }
            break;
        }

        case SYS_CLOSE: {
            int fd = (int)arg1;
            ret_val = vfs_close(fd);
            break;
        }
        
        case SYS_EXEC: {
            const char* filename_11 = (const char*)arg1; 

            if (filename_11 == 0) {
                ret_val = -EFAULT;
                break;
            }

            uint16_t start_cluster = find_file_in_root(filename_11);
            if (start_cluster == 0) {
                ret_val = -ENOENT;
                break; 
            }

            uint8_t* load_address = (uint8_t*)0x400000;
            uint32_t page_idx = (uint32_t)load_address / 0x400000; 
            uint32_t phys_addr = 0x400000; 

            page_directory[page_idx] = phys_addr | 0x83;

            __asm__ __volatile__("mov %%cr3, %%eax; mov %%eax, %%cr3" ::: "eax");

            read_file(filename_11, load_address);

            void (*program_entry)() = (void (*)())load_address;
            create_task(program_entry); 

            ret_val = 0; 
            break;
        }

        case SYS_TIME: {
            ret_val = (uint32_t)timer_ticks;
            break;
        }

        case SYS_GETPID: {
            ret_val = (uint32_t)current_task;
            break;
        }

        case SYS_UNAME: {
            char* user_buf = (char*)arg1;
            memcpy(user_buf, "CalcOS 10.4", 11);
            ret_val = 0;
            break;
        }

        case SYS_GET_BATTERY: {
            ret_val = (uint32_t)check_battery();
            break;
        }

        case SYS_GET_TICKS: {
            ret_val = (uint32_t)timer_ticks;
            break;
        }

        case SYS_PCI_PRINT: {
            pci_print_devices();
            ret_val = 0;
            break;
        }

        case SYS_LIST_FILES: {
            ret_val = vfs_list();
            break;
        }

        case SYS_WAIT_STRING: {
            char* user_buf = (char*)arg1;

            input_wait_string(user_buf);
            ret_val = 0;
            break;
        }

        case SYS_REBOOT: {
            reboot();
            ret_val = 0;
            break;
        }

        case SYS_GETUID: {
            ret_val = (uint32_t)current_uid;
            break;
        } 

        case SYS_WAIT_MULTILINE: {
            char* user_buf = (char*)arg1;

            input_wait_multiline(user_buf);
            ret_val = 0;
            break;
        }

        default:
            ret_val = -ENOSYS; 
            break;
    }
    regs->eax = ret_val; 
    return ret_val;
}
