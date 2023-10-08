#include <malloc.h>
#include "debug.h"

uint32_t getHeapAllocateSize(void) {
    struct mallinfo info = mallinfo();
    return info.uordblks;
}

uint32_t* __attribute__((target("arm"))) __attribute__((naked)) get_sp( ) {
    __asm volatile("mov r0,sp");
    __asm volatile("bx lr");
}
