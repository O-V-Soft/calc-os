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
#include <mm.h>

struct Stack {
    int *data;
    int size;
    int sp;
};

struct Word {
    const char* name;
    void (*fn)(struct Stack*);
};

struct Stack* stack_init(int size) {
    struct Stack* st = kmalloc(sizeof(struct Stack));
    if (!st) {
        print("\nforth: cannot allocate Stack\n", COLOR_WHITE);
    }

    st->size = size;
    st->sp = -1;

    st->data = kmalloc(size * sizeof(int));
    if (!st->data) {
        print("\nforth: cannot allocate stack data\n", COLOR_WHITE);
    }
        
    return st;
}

void stack_push(struct Stack* st, int value) {
    if (st->sp == st->size - 1) {
        int new_size = st->size + 32;
        int* new_data = kmalloc(new_size * sizeof(int));
        if (!new_data) {
            print("\nforth: cannot expand stack\n", COLOR_WHITE);
        }

        for (int i = 0; i <= st->sp; i++) {
            new_data[i] = st->data[i];
        }

        kfree(st->data);

        st->data = new_data;
        st->size = new_size;
    }

    st->data[++st->sp] = value;
}

int stack_pop(struct Stack* st) {
    if (st->sp == -1) {
        print("\nforth: pop from empty stack\n", COLOR_WHITE);
        return 0;
    }

    return st->data[st->sp--];
}

void word_add(struct Stack* st) {
    int b = stack_pop(st);
    int a = stack_pop(st);

    stack_push(st, a + b);
}

void word_sub(struct Stack* st) {
    int b = stack_pop(st);
    int a = stack_pop(st);

    stack_push(st, a - b);
}

void word_mul(struct Stack* st) {
    int b = stack_pop(st);
    int a = stack_pop(st);

    stack_push(st, a * b);
}

void word_div(struct Stack* st) {
    int b = stack_pop(st);
    int a = stack_pop(st);

    if (b == 0) {
        print("\nforth: division by zero\n", COLOR_WHITE);
        stack_push(st, 0);
    } else {
        stack_push(st, a / b);
    }
}

void word_dot(struct Stack* st) {
    int a = stack_pop(st);

    char buf[32];
    itoa(a, buf);  

    print(buf, COLOR_WHITE);
    print(" ", COLOR_WHITE); 
}

void word_fetch(struct Stack* st) {
    int addr = stack_pop(st);
    int value = *(int*)addr;

    stack_push(st, value);
}

void word_store(struct Stack* st) {
    int addr = stack_pop(st);
    int value = stack_pop(st);

    *(int*)addr = value;
}

void word_cr(struct Stack* st) {
    print("\n", COLOR_WHITE);
}

void word_equals(struct Stack* st) {
    int b = stack_pop(st);
    int a = stack_pop(st);

    stack_push(st, (a == b) ? -1 : 0);
}

void word_less(struct Stack* st) {
    int b = stack_pop(st);
    int a = stack_pop(st);

    stack_push(st, (a < b) ? -1 : 0);
}

void word_greater(struct Stack* st) {
    int b = stack_pop(st);
    int a = stack_pop(st);
    
    stack_push(st, (a > b) ? -1 : 0);
}

struct Word dictionary[] = {
    { "+",     word_add },
    { "-",     word_sub },
    { "*",     word_mul },
    { "/",     word_div },
    
    { "@",     word_fetch },   
    { "!",     word_store },  
    
    { ".",     word_dot },     
    { "CR",    word_cr },   
        
    { "=",     word_equals },
    { "<",     word_less },    
    { ">",     word_greater }  
};

struct Word* find_word(const char* token) {
    int count = sizeof(dictionary) / sizeof(dictionary[0]);

    for (int i = 0; i < count; i++) {
        if (compare_strings(token, dictionary[i].name)) {
            return &dictionary[i];
        }
    }

    return NULL;
}

char* next_token(char** input) {
    char* s = *input;

    while (*s == ' ' || *s == '\t') {
        s++;
    }

    if (*s == '\0') {
        *input = s;
        return NULL;
    }

    char* start = s;

    while (*s && *s != ' ' && *s != '\t') {
        s++;
    }
    
    if (*s) {
        *s = '\0';
        s++;
    }

    *input = s;
    return start;
}

int is_number(const char* s) {
    if (*s == '\0') return 0;

    if (*s == '-') s++;

    if (*s == '\0') return 0;

    while (*s) {
        if (*s < '0' || *s > '9')
            return 0;
        s++;
    }

    return 1;
}

void interpret(struct Stack* st, char* input) {
    char* ptr = input;
    char* token;

    while ((token = next_token(&ptr)) != NULL) {

        struct Word* w = find_word(token);
        if (w) {
            w->fn(st);
            continue;
        }

        if (is_number(token)) {
            int value = atoi(token);
            stack_push(st, value);
            continue;
        }

        print("\nforth: unknown word: ", COLOR_WHITE);
        print(token, COLOR_WHITE);
        print("\n", COLOR_WHITE);
    }

    delete_task(2);
}
