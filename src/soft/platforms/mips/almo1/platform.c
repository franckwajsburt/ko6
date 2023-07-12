#include <drivers/timer/soclib-timer.h>
#include <drivers/icu/soclib-icu.h>
#include <drivers/tty/soclib-tty.h>
#include <drivers/dma/soclib-dma.h>
#include <kernel/kthread.h>
#include <hal/cpu/irq.h>

struct timer_s timer;
extern int __timer_regs_map;

struct icu_s icu;
extern int __icu_regs_map;

struct tty_s tty0;
struct tty_s tty1;
extern int __tty_regs_map;

struct dma_s dma;
extern int __dma_regs_map;

void arch_init(int tick)
{
    // Initialize soclib ICU
    soclib_icu_ops.icu_init(&icu, __icu_regs_map);

    // Initialize timer and set the event to thread_yield, which
    // will cause a schedule each ntick
    soclib_timer_ops.timer_init(&timer, __timer_regs_map, tick);
    // TODO: this is ugly, fix it
    timer.ops->timer_set_event(&timer, (void (*)(void *)) thread_yield, (void *) 0);

    // Enable timer interrupts
    icu.ops->icu_unmask(&icu, 0);
    register_interrupt(0, (isr_t) soclib_timer_isr, &timer);

    // Initialize two ttys and register therm
    soclib_tty_ops.tty_init(&tty0, __tty_regs_map, 0);
    icu.ops->icu_unmask(&icu, 10);
    register_interrupt(10, (isr_t) soclib_tty_isr, &tty0);
    tty_add(&tty0);
    
    soclib_tty_ops.tty_init(&tty1, __tty_regs_map + 0x10, 0);
    icu.ops->icu_unmask(&icu, 11);
    register_interrupt(11, (isr_t) soclib_tty_isr, &tty1);
    tty_add(&tty1);

    // Initialize the DMA
    soclib_dma_ops.dma_init(&dma, __dma_regs_map);
}

void isrcall()
{
    int irq = icu.ops->icu_get_highest(&icu);
    route_interrupt(irq);
}