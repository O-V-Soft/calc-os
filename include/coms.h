#ifndef COMS_H
#define COMS_H
#include <stdint.h>

typedef struct {
    char* name;
    void (*func)();
} Command;

extern Command commands[];

int execute_command(char *line);

void cln();
void help();
void exit2();
void su();
void casm();
void empire();
void cat();
void touch();
void send();
void forth();

#endif