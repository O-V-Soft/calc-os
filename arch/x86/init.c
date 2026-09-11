#include <stdint.h>
#include <idt.h>
#include <task.h>

void init_paging();
void enable_paging();
void boot();
extern int current_mode;
extern void screen_clear();

void __attribute__((section(".text.entry"))) kernel_main() {
    init_paging();
    enable_paging();
    
    __asm__ __volatile__("cli");
	screen_clear();
    init_idt();
    __asm__ __volatile__("sti");
	
    boot();
    current_mode = 0;
	
	create_task(3);
	
	while (1) {
		__asm__ __volatile__("hlt");
	}
}
