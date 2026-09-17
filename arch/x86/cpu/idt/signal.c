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

void send_signal(int target_task_id, int signum) {
    if (signum <= 0 || signum >= NUM_SIGNALS) return;

    if (signum == SIGKILL) {
        delete_task(target_task_id); 
        return;
    }

    task_list[target_task_id].pending_signals = task_list[target_task_id].pending_signals | (1 << signum);
}

void check_signals(int current_task_id, uint32_t* registers_on_stack) {
    if (task_list[current_task_id].pending_signals == 0) return;

    for (int signum = 1; signum < NUM_SIGNALS; signum++) {
        if (task_list[current_task_id].pending_signals & (1 << signum)) {
            task_list[current_task_id].pending_signals = task_list[current_task_id].pending_signals & ~(1 << signum);

            if (signum == SIGINT) {
                delete_task(current_task_id);
                return;
            } 
            else if (signum == SIGILL) {
                delete_task(current_task_id);
                return;
            } 
            else if (signum == SIGFPE) {
                delete_task(current_task_id);
                return;
            } 
            else if (signum == SIGTERM) {
                delete_task(current_task_id);
                return;
            }
        }
    }
}
