#include <stdint.h>
#include <riscv.h>
#include <mm.h>
#include <coms.h>
#include <keyboard.h>
#include <utils.h>
#include <video.h>

char com[512];
char buffer[4096];

char name[128];
char content[512];

int current_uid = 1; 
const char* root_password = "102030!";

void system_riscv(uint32_t hartid, uint32_t dtb_ptr) {
    init_memory_manager();
    pci_scan();
	init_timer();
    init_gpu();
    screen_clear();
    
    while(1) {
        if (current_uid == 0) {
            print("# ", COLOR_WHITE);
        } else {
            print("$ ", COLOR_WHITE);
        }

        input_wait_string(com);

        printk("\n", COLOR_WHITE);

        char *com2 = strtok(com, " ");

        if (com2 != NULL) {
            int result = execute_command(com);
            if (result == 0) {
                continue;
			}            
            else {
                if (com[0] != '\0') {
                    printk("Unknown command. Type 'help'\n", COLOR_WHITE);
                }
            }
        }
	};
}
 
