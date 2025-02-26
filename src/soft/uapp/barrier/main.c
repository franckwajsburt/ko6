/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-02-17
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     barrier/main.c
  \author   Franck Wajsburt
  \brief    test of barriers

\*------------------------------------------------------------------------------------------------*/

#include <libc.h>
#include <pthread.h>

#define DELAY(n) for(int i=n;i--;) __asm__("nop");

pthread_t t0, t1;
struct arg_s a0, a1;
pthread_barrier_t barrier;

struct arg_s {
    int delay;
    char *message;
};


void * t0_fun (void * arg)
{
    struct arg_s * a = arg;
    for (int i=0;1;i++) {
        fprintf (1, "[%d] %s\n", i, a->message);
        DELAY ((a->delay) * (1 + rand()%2));
        pthread_barrier_wait (&barrier);
    };
}

int main (void)
{

    pthread_barrier_init (&barrier, NULL, 3);

    a0.delay = 100000;
    a0.message = "bonjour";

    a1.delay = 500000;
    a1.message = "salut";

    pthread_create (&t0, NULL, t0_fun, &a0);
    pthread_create (&t0, NULL, t0_fun, &a1);

    for (int i=0;1;i++) {
        fprintf (1, "[%d] app is alive\n", i);
        DELAY(100000);
        pthread_barrier_wait( &barrier);
    }

    void * trash;
    pthread_join (t1, &trash);
    pthread_join (t0, &trash);
    return 0;
}
