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

uint32_t syscall_handler(struct registers *regs) {
    uint32_t syscall_num = regs->eax;
    uint32_t arg1 = regs->ebx;
    uint32_t arg2 = regs->ecx;
    uint32_t arg3 = regs->edx;

    uint32_t ret_val = 0;

    switch (syscall_num) {
        case SYS_EXIT:
            delete_task(current_task);
            ret_val = 0;
            break;

        case SYS_OPEN: {
            const char* path = (const char*)arg1;
            if (path < (const char*)(13 * 4096)) {
                ret_val = -EFAULT;
                break;
            }
            ret_val = vfs_open(path);   
            break;
        }

        case SYS_READ: {
            int fd = (int)arg1;
            char* buf = (char*)arg2;
            size_t count = (size_t)arg3;

            if (buf < (char*)(13 * 4096)) {
                ret_val = -EFAULT;
                break;
            }

            if (fd == 0) {
                if (count == 0) {
                    ret_val = 0;
                } else {
                    uint8_t scancode = get_scancode();
                    if (scancode != 0) {
                        buf[0] = scancode;
                        ret_val = 1;
                    } else {
                        ret_val = 0;
                    }
                }
            } else {
                ret_val = vfs_read(fd, buf, count);
            }

            break;
        }
        
        case SYS_WRITE: {
            if (arg1 == 1 || arg1 == 2) {
                printk((char*)arg2, (uint8_t)arg3);
                ret_val = 0;
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

            if (filename_11 == 0) ret_val = -EFAULT;

            uint16_t start_cluster = find_file_in_root(filename_11);
            if (start_cluster == 0) ret_val = -ENOENT;

            uint8_t* load_address = (uint8_t*)0x400000;
            
            uint32_t page_idx = (uint32_t)load_address / 0x400000; 
            
            uint32_t phys_addr = 0x400000; 

            page_directory[page_idx] = phys_addr | 0x83;

            __asm__ __volatile__("mov %%cr3, %%eax; mov %%eax, %%cr3" ::: "eax");

            memset((void*)0x400000, 0x90, 4096);

            read_file(filename_11, load_address);

            __asm__ __volatile__("cli");

            void (*program_entry)(void) = (void (*)(void))load_address;
            program_entry(); 

            __asm__ __volatile__("sti");

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
            if (user_buf < (char*)(13 * 4096)) ret_val = -EFAULT;
            memcpy(user_buf, "CalcOS 10.2", 9);
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
            ret_val = vfs_list("/");
            break;
        }

        case SYS_CREATE_FILE: {
            const char* name_11 = (const char*)arg1;
            uint8_t* buffer = (uint8_t*)arg2;
            uint32_t length = arg3;
            if (name_11 == 0 || buffer == 0) ret_val = -EFAULT;
            create_file((char*)name_11, buffer, (int)length);
            ret_val = 0;
            break;
        }

        case SYS_WAIT_STRING: {
            char* user_buf = (char*)arg1;
            if (user_buf < (char*)(13 * 4096)) ret_val = -EFAULT;

            input_wait_string(user_buf);
            ret_val = 0;
            break;
        }

        case SYS_REBOOT: {
            reboot();
            ret_val = 0;
            break;
        }

        case SYS_BEHAVE: {
            read_pack();
            ret_val = 0;
            break;
        }

        case SYS_MOUNT: {
            const char* path = (const char*)arg1;
            if (path < (const char*)(13 * 4096)) ret_val = -EFAULT;

            int res = vfs_mount(path, &fat12_driver);
            ret_val = res == 0 ? 0 : -EINVAL;
            break;
        }

        case SYS_UMOUNT2: {
            const char* path = (const char*)arg1;
            if (path < (const char*)(13 * 4096)) ret_val = -EFAULT;

            int res = vfs_umount(path);
            ret_val = res == 0 ? 0 : -EINVAL;
            break;
        }

        case SYS_GETUID: {
            ret_val = (uint32_t)current_uid;
            break;
        } 

        case SYS_GET_COLOR: {
            ret_val = COLOR_WHITE;
            break;
        }

        default:
            ret_val = -ENOSYS; 
            break;
    }
    regs->eax = ret_val; 
    return ret_val;
}
