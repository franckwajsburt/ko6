#include <libc.h>


int main() {
    fprintf(1, "hello, friend\n");
    syscall_fct(42, 42, 42, 42, SYSCALL_KSHELL);

    return 0;
}