#include <coms.h>
#include <riscv.h>
#include <video.h>
#include <stdint.h>
#include <keyboard.h>
#include <utils.h>

#if defined(__riscv)
Command commands[] = {
    {"cln", cln},
    {"help", help},
    {"exit", exit2},
    {"su", su},
    {"lspci", devices2},
    {"lifetime", lifetime},
    {"whoami", whoami},
    {"status", status},
    {"reset", reset},
    {NULL, NULL}
};
#else
void sys_wait_string(char* buf);
Command commands[] = {
    {"cln", cln},
    {"help", help},
    {"exit", exit2},
    {"su", su},
    {"casm", casm},
    {"empire", empire},
    {"cat", cat},
    {"touch", touch},
    {"ping", ping},
    {"forth", forth},
    {"cube", cube},
    {NULL, NULL}
};
#endif

int execute_command(char *line) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(line, commands[i].name) == 0) {
            commands[i].func();
            return 0;
        }
    }

    return -1;
}

void cln() {
    #if defined(__riscv)
        screen_clear();
	ncount = 1;
	is_scaled = 0;
        return;
    #else
        screen_clear();
        ncount = 1;
        return;
    #endif
}

void help() {
    #if defined(__riscv)
    print("Available commands:\n");
    print("  help - show this message\n");
    print("  status - print CPU status\n");
    print("  reset - reset the system\n");
    print("  lspci - print PCI devices\n");
    print("  lifetime - system timer ticks\n");
    print("  su - change to root\n");
    print("  exit - change to user\n");
    print("  whoami - print current user\n");
    #else
    print("Available commands:\n");
    print("  help - show this message\n");
    print("  cln  - clear the screen\n");
    print("  ls  - list all files\n");
    print("  touch  - create a new file\n");
    print("  status - check system status\n");
    print("  cat - print file content\n");
    print("  lspci - print PCI devices\n");
    print("  ping - ping the network\n");
    print("  reboot - reboot the system\n");
    print("  forth - Forth interpreter\n");
    print("  empire - play imperial march\n");
    print("  umount - umount FS\n");
    print("  mount - mount FS\n");
    print("  casm - Calc Assembly Compiler\n");
    print("  su - change to root\n");
    print("  exit - change to user\n");
    print("  whoami - print current user\n");
    print("  cube - spin the cube\n");
    #endif
}

void exit2() {
    if (current_uid == 0) {
        current_uid = 1;
    } 
}

void su() {
    if (current_uid == 0) {
        return;
    } else {
        char pass_buf[32];
        print("Password: ");
        input_wait_string(pass_buf);
        print("\n");

        if (compare_strings(pass_buf, (char*)root_password)) {
            current_uid = 0;
        } else {
            print("su: Authentication failure\n");
        }
    }
}
