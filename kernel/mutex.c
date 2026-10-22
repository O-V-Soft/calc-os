#include <stdint.h>
#include <mm.h>

void acquire_lock(volatile int *lock) {
    while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE));
}

void release_lock(volatile int *lock) {
    __atomic_clear(lock, __ATOMIC_RELEASE);
}
