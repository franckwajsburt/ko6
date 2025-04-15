/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/soc/almo1-mips/soc.c
  \author   Franck Wajsburt, Nolan Bled
  \brief    Platform initialization functions
            Initialize every platform device based on the device tree

            This file is the glue between the device tree and the drivers.
            It knows:
            - the hardware platform,
            - the offsets in the FDT,
            - and it calls the right initialization functions for each driver.

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/blockdev/soclib-bd.h>
#include <hal/devices/chardev/soclib-tty.h>
#include <hal/devices/timer/soclib-timer.h>
#include <hal/devices/icu/soclib-icu.h>
#include <hal/devices/dma/soclib-dma.h>

#include <hal/cpu/irq.h>

#include <kernel/kthread.h>
#include <kernel/klibc.h>
#include <kernel/kirq.h>

#include <external/libfdt/libfdt.h>

/**
 * \brief   Get the base address of a FDT device node (reg property)
 *          TODO: take in account the cells attribute of a node
 * \param   fdt address of the FDT in memory 
 * \param   offset offset of the node in the FDT
 * \return  the address contained by the reg property
 */
static unsigned get_base_address(void *fdt, int offset)
{
    return fdt32_to_cpu(
        *(unsigned *) fdt_getprop(fdt, offset, "reg", NULL));
}

/**
 * \brief   Get the IRQ of a FDT device node (interrupts property)
 *          TODO: handle multiple IRQs per node
 * \param   fdt address of the FDT in memory 
 * \param   offset offset of the node in the FDT
 * \return  the IRQ contained by the interrupts property
 */
static unsigned get_irq(void *fdt, int offset)
{
    return fdt32_to_cpu(
        *(unsigned *) fdt_getprop(fdt, offset, "interrupts", NULL));
}

/**
 * \brief   Initialize ICUs described by the device tree
 *          See the soc_tty_init function for a detailed explanation
 * \param   fdt fdt address
 * \return  nothing
 */
static void soc_icu_init(void *fdt)
{
    int icu_off = fdt_node_offset_by_compatible(fdt, -1, "soclib,icu");
    while (icu_off != -FDT_ERR_NOTFOUND) {
        unsigned addr = get_base_address(fdt, icu_off);

        struct icu_s *icu = icu_alloc();
        SoclibICUOps.icu_init(icu, addr);

        icu_off = fdt_node_offset_by_compatible(fdt, icu_off, "soclib,icu");
    }
}

/**
 * \brief   Initialize the TTYs present on the SOC based on the device-tree content
 *          Basically, every device initialization follow the same process:
 *          We loop on each device compatible with our drivers for this platform, e.g. soclib,tty
 *          For each device, we find it's base address in the reg property of the device tree node
 *          and its IRQ in the interrupts property.
 *          Once we've fetched this data, we allocate a new device (see hal/dev.h) and register it
 *          in the device list.
 *          We then setup the dev with a device-specific function declared by the HAL (ex: tty_init)
 *          Last thing we do is unmask the interrupt in the ICU and register the ISR for the device.
 * \param   fdt fdt address
 * \return  -1 if the initialization failed, 0 if it succeeded
 */
static int soc_tty_init(void *fdt)
{
    // Fetch the ICU device
    struct icu_s *icu = icu_get(0);
    // Check that the ICU has already been initialized
    if (!icu)
        return -1;

    // Find the first TTY device
    int tty_off = fdt_node_offset_by_compatible(fdt, -1, "soclib,tty");

    // Loop until we can't find any more compatible ttys in the device tree
    while (tty_off != -FDT_ERR_NOTFOUND) {
        // Fetch the necessary properties from the device tree
        unsigned addr = get_base_address(fdt, tty_off);
        unsigned irq = get_irq(fdt, tty_off);

        // Allocate the structure and add it in the global device list
        struct chardev_s *tty = chardev_alloc();
        // Initialize the device
        SoclibTTYOps.chardev_init(tty, addr, 0);
        // Unmask the interrupt
        icu->ops->icu_unmask(icu, irq);
        // Register the corresponding ISR
        register_interrupt(irq, (isr_t) soclib_tty_isr, tty);

        // Find the next compatible tty
        tty_off = fdt_node_offset_by_compatible(fdt, tty_off, "soclib,tty");
    }

    return 0;
}

/**
 * \brief   Initialize timers described by the device tree
 *          See the soc_tty_init function for a detailed explanation
 * \param   fdt fdt address
 * \param   tick number of ticks between two timer interrupts
 * \return  -1 if the initialization failed, 0 if it succeeded
 */
static int soc_timer_init(void *fdt, unsigned tick)
{
    // Fetch the ICU device
    struct icu_s *icu = icu_get(0);
    if (!icu)
        return -1;

    int timer_off = fdt_node_offset_by_compatible(fdt, -1, "soclib,timer");

    while (timer_off != -FDT_ERR_NOTFOUND) {
        unsigned addr = get_base_address(fdt, timer_off);
        unsigned irq = get_irq(fdt, timer_off);

        struct timer_s *timer = timer_alloc();
        SoclibTimerOps.timer_init(timer, addr, tick);
        timer->ops->timer_set_event(timer,
            (void (*)(void *)) tick_event, (void *) 0);
        //  (void (*)(void *)) thread_yield, (void *) 0);

        icu->ops->icu_unmask(icu, irq);
        register_interrupt(irq, (isr_t) soclib_timer_isr, timer);

        timer_off = fdt_node_offset_by_compatible(fdt, timer_off, "soclib,timer");
    }

    return 0;
}

/**
 * \brief   Initialize dmas described by the device tree
 *          See the soc_tty_init function for a detailed explanation
 * \param   fdt fdt address
 * \return  nothing
 */
static void soc_dma_init(void *fdt)
{
    // TODO: handle DMA interrupts
    int dma_off = fdt_node_offset_by_compatible(fdt, -1, "soclib,dma");
    while (dma_off != -FDT_ERR_NOTFOUND) {
        unsigned addr = get_base_address(fdt, dma_off);

        struct dma_s *dma = dma_alloc();
        SoclibDMAOps.dma_init(dma, addr);

        dma_off = fdt_node_offset_by_compatible(fdt, dma_off, "soclib,dma");
    }
}

/**
 * \brief   Initialize block device described by the device tree
 *          See the soc_ty_init function for a detailed explanation
 * \param   fdt fdt address
 * \return  -1 if the initialization failed, 0 if it succeeded
 */
static int soc_bd_init(void *fdt)
{
    // Fetch the ICU device
    struct icu_s *icu = icu_get(0);
    if (!icu)
        return -1;

    int bd_off = fdt_node_offset_by_compatible(fdt, -1, "soclib,bd");

    if (bd_off == -FDT_ERR_NOTFOUND) 
        return -1;

    unsigned addr = get_base_address(fdt, bd_off);
    unsigned irq = get_irq(fdt, bd_off);

    struct blockdev_s *bd = blockdev_alloc();
    SoclibBDOps.blockdev_init(bd, addr, LOGICAL_BLOCK_SIZE);

    icu->ops->icu_unmask(icu, irq);
    register_interrupt(irq, (isr_t) soclib_bd_isr, bd);

    bd_off = fdt_node_offset_by_compatible(fdt, bd_off, "soclib,db");

    return 0;
}

/**
 * \brief For the SoC almo1, IRQ n'x (that is ICU.PIN[x]) is wired to the IRQ Signal of device n'y
 *        There are at most 14 IRQs for almo1, but the real number depends of the SoC paramaters
 *        There are as many timers as CPU, thus NCPUS timers
 *        There are Nicus ttys
 *        There are also a DMA to perfom memcpy and optionnal Block Device (hard drive)
 *        
 *        Device IRQs are wired as following:
 *        * ICU.PIN [0]  : timer 0
 *             '     '        '        depending on NCPUS (0 to 7)
 *        * ICU.PIN [7]  : timer 7
 *        * ICU.PIN [8]  : bd          Bloc Device (Hard Drive)
 *        * ICU.PIN [9]  : dma         Direct Memory Access (Hard memcpy)
 *        * ICU.PIN [10] : TTY0        TTY n'0
 *             '     '      '          depending on NTTYS (0 to 3)
 *        * ICU.PIN [13] : TTY3
 *
 * \param   tick    number of ticks between two timer interrupts
 * \return  -1 if the initialization failed, 0 if it succeeded
 */
int soc_init(void *fdt, int tick)
{
    if (fdt_magic(fdt) != 0xd00dfeed)
        return -1;

    // We MUST initialize the ICU first since every other device
    // initialization will rely on it for its interrupts
    soc_icu_init(fdt);

    // Initialize TTY early so we can debug as soon as possible
    if (soc_tty_init(fdt) < 0)
        return -1;

    soc_bd_init(fdt);
    soc_dma_init(fdt);

    // Finish by the timer (we don't want to schedule anything until everything
    // is initialized)
    if (soc_timer_init(fdt, tick) < 0)
        return -1;

    return 0;
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

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
