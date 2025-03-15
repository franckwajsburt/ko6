#include <kshell.h>

void sys_kshell(int a0, int a1, int a2, int a3) {
    kprintf("hello from sys_kshell!\n");
    kprintf("a0: %d, a1: %d, a2: %d, a3: %d\n", a0, a1, a2, a3);
}