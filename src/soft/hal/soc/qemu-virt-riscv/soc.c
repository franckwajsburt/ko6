/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-08-01
  | / /(     )/ _ \     \copyright  2021-2023 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/soc/qemu-virt-riscv/soc.c
  \author   Nolan Bled
  \brief    Platform initialization functions

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/timer/clint-timer.h>
#include <hal/devices/chardev/ns16550.h>
#include <hal/devices/icu/plic.h>

#include <kernel/kthread.h>
#include <kernel/klibc.h>
#include <kernel/kirq.h>

#include <hal/cpu/irq.h>

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
    /* We fetch the second element since address-cells = 2 */
    return fdt32_to_cpu(
        ((unsigned *) fdt_getprop(fdt, offset, "reg", NULL))[1]);
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
    int icu_off = fdt_node_offset_by_compatible(fdt, -1, "riscv,plic0");
    while (icu_off != -FDT_ERR_NOTFOUND) {
        unsigned addr = get_base_address(fdt, icu_off);

        struct icu_s *icu = icu_alloc();
        PlicOps.icu_init(icu, addr);

        icu_off = fdt_node_offset_by_compatible(fdt, icu_off, "riscv,plic0");
    }
}

/**
 * \brief   Initialize the TTYs present on the SOC based on the device-tree content
 *          Basically, every device initialization follow the same process:
 *          We loop on each device compatible with our drivers for this platform, by exemple soclib,tty
 *          For each device, we find it's base address in the reg property of the device tree node
 *          and its IRQ in the interrupts property.
 *          Once we've fetched this data, we allocate a new device (see hal/dev.h) and register it
 *          in the device list.
 *          We then setup the device with a device-specific function declared by the HAL (ex: tty_init)
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
    int tty_off = fdt_node_offset_by_compatible(fdt, -1, "ns16550a");

    // Loop until we can't find any more compatible ttys in the device tree
    while (tty_off != -FDT_ERR_NOTFOUND) {
        // Fetch the necessary properties from the device tree
        unsigned addr = get_base_address(fdt, tty_off);
        unsigned irq = get_irq(fdt, tty_off);

        // Allocate the structure and add it in the global device list
        struct chardev_s *tty = chardev_alloc();
        // Initialize the device
        NS16550Ops.chardev_init(tty, addr, 9600);
        // Unmask the interrupt
        icu->ops->icu_unmask(icu, irq);
        icu->ops->icu_set_priority(icu, irq, 1);

        // Register the corresponding ISR
        register_interrupt(irq, (isr_t) ns16550_isr, tty);

        // Find the next compatible tty
        tty_off = fdt_node_offset_by_compatible(fdt, tty_off, "ns16550a");
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

    int timer_off = fdt_node_offset_by_compatible(fdt, -1, "sifive,clint0");

    while (timer_off != -FDT_ERR_NOTFOUND) {
        unsigned addr = get_base_address(fdt, timer_off);

        struct timer_s *timer = timer_alloc();
        ClintTimerOps.timer_init(timer, addr, tick);
        timer->ops->timer_set_event(timer,
            (void (*)(void *)) thread_yield, (void *) 0);

        timer_off = fdt_node_offset_by_compatible(fdt, timer_off, "sifive,clint0");
    }

    return 0;
}

/**
 * \brief   QEMU virt platform initialization
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

    // Finish by the timer (we don't want to schedule anything until everything
    // is initialized)
    // TODO: find a way to make tick portable
    (void) tick;
    if (soc_timer_init(fdt, 10000000) < 0)
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
void isrcall(uint32_t mcause)
{
    // Check if this a timer or an external interrupt
    mcause &= ~(1 << 31);
    if (mcause == 7) {
        // Machine Timer interrupt
        struct timer_s *timer = timer_get(0); // TODO: see if we should use cpuid() here instead of 0
        clint_timer_isr(0, timer);
    } else if (mcause == 11) {
        // Machine external interrupt
        struct icu_s *icu = icu_get(cpuid());           // get the ICU which has dev.no == cpuid()
        int irq = icu->ops->icu_get_highest(icu);       // IRQ nb with the highest prio
        route_interrupt(irq);                           // launch the ISR for the bound device
        icu->ops->icu_acknowledge(icu, irq);
    }
}
