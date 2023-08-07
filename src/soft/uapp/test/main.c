#include <libc.h>
#include <pthread.h>

int main (void)
{
    unsigned a = 0xdeadbeef;
    unsigned b = 0xcafebabe;
    unsigned char c = 'A';
    const char *d = "Hey! I’m no one’s messenger boy, all right? I’m a delivery boy."; 
    fprintf(1, "a=0x%x b=%d c='%c' d=\"%s\"\n",
        a, b, c, d);
    return 0;
}