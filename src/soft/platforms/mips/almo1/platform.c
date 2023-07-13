#include <drivers/timer/soclib-timer.h>
#include <drivers/icu/soclib-icu.h>
#include <drivers/tty/soclib-tty.h>
#include <drivers/dma/soclib-dma.h>
#include <kernel/kthread.h>
#include <hal/cpu/irq.h>

/**
 * Constants defined in the kernel linker script that allows us to know where the device
 * are memory-mapped
*/
extern int __timer_regs_map;
extern int __icu_regs_map;
extern int __tty_regs_map;
extern int __dma_regs_map;

/**
 * For the SoC almo1, IRQ n'x (that is ICU.PIN[x]) is wired to the Interrup Signal of device n'y
 *
 * There are at most 14 IRQs for almo1, but the real number depends of the prototype paramaters
 * There are as many timers as CPU, thus NCPUS timers
 * There are NTTYs ttys
 * There are also a DMA to perfom memcpy and optionnal Block Device (hard drive)
 *
 * Device IRQs are wired as following:
 * * ICU.PIN [0]  : timer 0
 *      "     "        "        depending on NCPUS (0 to 7)
 * * ICU.PIN [7]  : timer 7
 * * ICU.PIN [8]  : bd          Bloc Device (Hard Drive)
 * * ICU.PIN [9]  : dma         Direct Memory Access (Hard memcpy)
 * * ICU.PIN [10] : TTY0        TTY n'0
 *      "     "      "          depending on NTTYS (0 to 3)
 * * ICU.PIN [13] : TTY3
 */
void arch_init(int tick)
{
    // Initialize soclib ICU
    struct icu_s *icu = icu_alloc();
    soclib_icu_ops.icu_init(icu, (unsigned) &__icu_regs_map);

    // Initialize timer and set the event to thread_yield, which
    // will cause a schedule each ntick
    struct timer_s *timer = timer_alloc();
    soclib_timer_ops.timer_init(timer, (unsigned) &__timer_regs_map, tick);
    timer->ops->timer_set_event(timer, (void (*)(void *)) thread_yield, (void *) 0);

    // Enable timer interrupts
    icu->ops->icu_unmask(icu, 0);
    register_interrupt(0, (isr_t) soclib_timer_isr, timer);

    // Initialize two ttys and register therm
    struct tty_s *tty = tty_alloc();
    soclib_tty_ops.tty_init(tty, (unsigned) &__tty_regs_map, 0);
    icu->ops->icu_unmask(icu, 10);
    register_interrupt(10, (isr_t) soclib_tty_isr, tty);
    
    tty = tty_alloc();
    soclib_tty_ops.tty_init(tty, (unsigned) &__tty_regs_map + 0x10, 0);
    icu->ops->icu_unmask(icu, 11);
    register_interrupt(11, (isr_t) soclib_tty_isr, tty);

    // Initialize the DMA
    struct dma_s *dma = dma_alloc();
    soclib_dma_ops.dma_init(dma, (unsigned) &__dma_regs_map);
}

/**
 * \brief   This function calls the ISR of the highest priority IRQ
 *          It is called by the irq_handler routine when a CPU is interrupted by an IRQ
 *          Its aims is to find out which IRQ is now active to know which device needs the CPU.
 *          and to launch the right ISR of the right device instance by using IRQVector tables.
 * TODO: maybe this should also be a "standard" function, declared in a HAL file
 */
void isrcall()
{
    struct icu_s *icu = icu_get(cpuid());           // get the ICU which has dev.no == cpuid()
    int irq = icu->ops->icu_get_highest(icu);       // IRQ nb with the highest prio
    route_interrupt(irq);                           // launch the ISR for the bound device
}