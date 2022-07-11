/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/almo1/harch.c
  \author   Franck Wajsburt
  \brief    driver code for all devices which implement harch.h API
            - Devices are : dma, tty, timer, icu 
            - The devices register map is used only in this file

\*------------------------------------------------------------------------------------------------*/

#include <klibc.h>      

//--------------------------------------------------------------------------------------------------
// DMA Definition
// - struct dma_s       dma's registers map description
// - __dma_regs_map     declare dma's registers map defined in ldscript
// - volatile           means that compiler must really read from and write to memory
//                      thus it must not use registers to optimize
//--------------------------------------------------------------------------------------------------

struct dma_s {
    void * src;         // dma's destination buffer address
    void * dest;        // dma's source buffer address
    int len;            // number of bytes to move
    int reset;          // IRQ acknowledgement
    int irq_enable;     // IRQ mask
    int unused[3];      // unused addresses
};

extern volatile struct dma_s __dma_regs_map;

//--------------------------------------------------------------------------------------------------

void * dma_memcpy (int *dest, int *src, unsigned n)
{
    dcache_buf_invalidate(dest,n);  // if there are lines of this buffer in the cache, forget them
    __dma_regs_map.dest = dest;     // destination address
    __dma_regs_map.src = src;       // source address
    __dma_regs_map.len = n;         // at last number of byte
    while (__dma_regs_map.len) delay (100);
    return dest;
}

//--------------------------------------------------------------------------------------------------
// TTY Definition
// - NTTYS              is the number of available TTY
// - struct tty_s       tty's registers map description
// - __tty_regs_map     declare tty's registers map defined in ldscript
// - volatile           means that compiler must really read from and write to memory
//                      thus it must not use registers to optimize
//--------------------------------------------------------------------------------------------------

struct tty_s {
    int write;          // tty's output address
    int status;         // tty's status address something to read if not null)
    int read;           // tty's input address
    int unused;         // unused address
};

extern volatile struct tty_s __tty_regs_map[NTTYS];

//--------------------------------------------------------------------------------------------------

/**
 * Simple fifo (1 writer - 1 reader)
 *   - data      buffer of data
 *   - pt_write  write pointer for L fifos (0 at the beginning)
 *   - pt_read   read pointer for L fifos (0 at the beginning)
 *
 * data[] is used as a circular array. At the beginning (pt_read == pt_write) means an empty fifo
 * then when we push a data, we write it at pt_write, the we increment pt_write % fifo_size.
 * The fifo is full when it remains only one free cell, then when (pt_write + 1)%size == pt_read
 */
struct tty_fifo_s {
        char data [20];
        int  pt_read;
        int  pt_write;
};

/**
 * \brief array of fifos, one per tty, the writer is tty_isr(), the reader is tty_read()
 */
static struct tty_fifo_s TTYFifo [NTTYS];

/**
 * \brief   write to the FIFO
 * \param   fifo    structure of fifo to store data
 * \param   c       char to write
 * \return  SUCCESS or FAILURE
 */
static int tty_fifo_push (struct tty_fifo_s *fifo, int c)
{
    int pt_write_next = (fifo->pt_write + 1) % sizeof(fifo->data);
    if (pt_write_next != fifo->pt_read) {
        fifo->data [fifo->pt_write] = c;
        fifo->pt_write = pt_write_next;
        return SUCCESS;
    }
    return FAILURE;
}

/**
 * \brief   read from the FIFO
 * \param   fifo    structure of fifo to store data
 * \param   c       pointer on char to put the read char 
 * \return  SUCCESS or FAILURE
 */
static int tty_fifo_pull (struct tty_fifo_s *fifo, int *c)
{
    if (fifo->pt_read != fifo->pt_write) {
        *c = fifo->data [fifo->pt_read];
        fifo->pt_read = (fifo->pt_read + 1)% sizeof(fifo->data);
        return SUCCESS;
    }
    return FAILURE;
}

/**
 *  \brief     isr for tty IRQ, used by isrcall()
 *             we do not read the tty status register because we are sure there is a char to read.
 *             the character read is pushed into the fifo dedicated for the tty
 *             if the fifo is full, the character is lost.
 *  \param     tty device instance number
 */
static void tty_isr (int tty)
{
    struct tty_fifo_s *fifo = &TTYFifo[ tty%NTTYS ];    // dedicated fifo for this tty
    int c = __tty_regs_map[tty].read;                   // read the char
    tty_fifo_push (fifo, c);                            // push it the fifo
}

/**
 * tty_read should sleep, but it will be the case, when there will have read drivers
 */
int tty_read (int tty, char *buf, unsigned count)
{
    int res = 0;                                        // nb of read char
    tty = tty % NTTYS;                                  // to be sure that tty is an existing tty
    int c;                                              // char read
    struct tty_fifo_s *fifo = &TTYFifo[ tty%NTTYS ];    // dedicated fifo for this tty

    while (count--) {
        while (tty_fifo_pull (fifo, &c) == FAILURE) {   // wait for a char from the keyboard
            thread_yield();                             // nothing then we yield the processor
            irq_enable();                               // get few characters if thread is alone
            irq_disable();                              // close enter
        }
        *buf++ = c;
        res++;
    }
    return res;                                         // return the number of char read
}

int tty_write (int tty, char *buf, unsigned count)
{
    int res = 0;                                        // nb of written char
    tty = tty % NTTYS;                                  // to be sure that tty exists

    VAR("%d",tty);

    while (count--) {                                   // while there are chars
        __tty_regs_map[ tty ].write = *buf;             // send the char to TTY
        res++;                                          // nb of written char
        buf++;		                                    // but is the next address in buffer
    }
    return res;
}

//-------------------------------------------------------------------------------------------------
// TIMER Definition
// - NCPUS              is the number of available TIMERs given to gcc (see Makefile)
// - struct timer_s     timer's registers map description
// - __timer_regs_map   declare timer's registers map defined in ldscript
// - volatile           means that compiler must really read from and write to memory
//                      thus it must not use registers to optimize
//-------------------------------------------------------------------------------------------------

struct timer_s {
    int value;          // timer's counter : +1 each cycle, can be written
    int mode;           // timer's mode : bit 0 = ON/OFF ; bit 1 = IRQ enable
    int period;         // timer's period between two IRQ
    int resetirq;       // address to acknowledge the timer's IRQ
};

extern volatile struct timer_s __timer_regs_map[NCPUS];

//--------------------------------------------------------------------------------------------------

/**
 * \brief     start the timer to periodically raise an IRQ (only used by arch_init)
 * \param     timer  timer number (between 0 and NCPUS-1)
 * \param     tick   delay between two IRQ
 */
static void timer_init (int timer, int tick)
{
    timer = timer % NCPUS;                      // the number of timers is the number of CPU
    __timer_regs_map[timer].resetirq = 1;       // to be sure there won't be a IRQ when timer start
    __timer_regs_map[timer].period =  tick;     // next period
    __timer_regs_map[timer].mode = (tick)?3:0;  // timer ON with IRQ only if (tick != 0)
}

/**
 * \brief     isr for timer IRQ (only used by icu_init)
 * \param     timer device instance number
 */
static void timer_isr (int timer)
{
    __timer_regs_map[timer].resetirq = 1;       // IRQ acknoledgement to lower the interrupt signal
    thread_yield ();
}

//-------------------------------------------------------------------------------------------------
// ICU Definition
// - NCPUS              is the number of available ICUs given to gcc (see Makefile)
// - struct icu_s       icu's registers map description
// - __icu_regs_map     declare icu's registers map defined in ldscript
// - volatile           means that compiler must really read from and write to memory
//                      thus it must not use registers to optimize
//-------------------------------------------------------------------------------------------------

struct icu_s {
    int state;          // state of all IRQ signals
    int mask;           // IRQ mask to chose what we need for this ICU
    int set;            // IRQ set   --> enable specific IRQs for this ICU
    int clear;          // IRQ clear --> disable specific IRQs for this ICU
    int highest;        // highest pritority IRQ number for this ICU
    int unused[3];      // 3 register addresses are not used
};

extern volatile struct icu_s __icu_regs_map[NCPUS];

//--------------------------------------------------------------------------------------------------

/**
 * \brief     enable device IRQ siqnal
 * \param     icu  icu number (between 0 and NCPUS-1)
 * \param     irq  irq number to enable
 */
static void icu_set_mask (int icu, int irq)
{
    icu = icu % NCPUS;                      // the number of ICU is the number of CPU
    __icu_regs_map[icu].set = 1 << irq;     // set bit n'irq to 1, do not change the others
}

/**
 * \brief     get the highest priority irq number for an ICU instance
 *            There are as many ICU as CPU.
 *            Each ICU allows to select which IRQ must be handled by the corresponding CPU
 *            The IRQ selection is done thanks to the icu_set_mask() functioni.
 *            If there are several IRQs active at the same time, the register named "highest"
 *            gives the IRQ number with the highest priority
 * \param     icu  icu number (which is in fact the CPU number, thus between 0 and NCPUS-1)
 * \return    the highest priorty irq number that is for this ICU the lowest irq number
 */
static int icu_get_highest (int icu)
{
    icu = icu % NCPUS;                      // the number of ICU is the number of CPU
    return __icu_regs_map[icu].highest;     // get and return the  highest priority irq number
}

//-------------------------------------------------------------------------------------------------
// Architecture intialisation
//-------------------------------------------------------------------------------------------------

/**
 * isr_t is a type of an isr function
 *
 * This comment explains what is a type of function, and how to create one.
 * If we have a function : RETURN_TYPE FUNCTION_NAME ( list of ARG_TYPE ARG_NAME )
 *
 * For example : int erase (double * tab, unsigned size);
 *  * RETURN_TYPE = int          * FUNCTION_NAME = erase
 *  * ARG_TYPE 1  = double       * ARG_TYPE 2    = unsigned
 *  * ARG_NAME 1  = tab          * ARG_NAME 2    = size
 *
 * To create a variable named VF which is a pointer to a function, we must write :
 *    RETURN_TYPE (*VF) ( list of ARG_TYPE ARG_NAME );
 *    be warned that the parentheses around *VF are mandatory,
 *    otherwise gcc recognize a simple declaration of function
 *
 * For example, we can write :
 *    int (*VF) (double * tab, unsigned size); // declaration of VF
 *    VF = erase;                              // initialisation of VF
 *    int x = VF (tab, size);                  // this line is equivalent to
 *    int x = erase (tab, size);               // this one
 *
 * To create a type named TF for the pointer to a function, we must write :
 *    typedef RETURN_TYPE (*TF) ( list of ARG_TYPE ARG_NAME );
 *    the parentheses around *TF are also mandatory,
 *
 * For example, we can write :
 *    typedef int (*TF) (double * tab, unsigned size);
 *    TF VF ;
 *    VF = erase;
 *    int x = VF (tab, size);
 *    int x = erase (tab, size);
 */
typedef  void (*isr_t) (int);

/**
 * \brief   Default Interrupt Service Routine which will be executed if an unexpected IRQ happens
 */
static void isr_default (int irq)
{
    PANIC_IF(true,"ERROR: unexpected IRQ n'%d\n", icu_get_highest (cpuid()));
}

/**
 * The interrupt vector is a structure that binds the interrupt service routine to the interrupt
 * requests. Each IRQ may have its own ISR.
 *
 * An interrupt requets (IRQ) is an electrical signal comming from a device to warn of an event.
 * An Interrupt Service Routine (ISR) is a function that handles a specific IRQ.
 *
 * For each IRQ, we need to know which ISR handles it AND from which device it is come through.
 * That's why we have 2 tables here:
 * * IRQVectorISR[] given the ISR to execute for each IRQ
 *   For example: IRQVectorISR[13] = tty_isr; if the IRQ n'13 is bound to the tty device.
 * * IRQVectorDev[] given the device instance number from the IRQ.
 *   For example: IRQVectorDev[13] = 3; if the IRQ n'13 is bound to the tty device n'3.
 *   Because, we can have several tty, all of them share the ISR, but we need to know which
 *   instance in order to use the right device registers.
 *
 * Theses tables are filled in arch_init() with the knownledge of the SoC architecture.
 */
static isr_t IRQVectorISR[32] = { [0 ... 31] = isr_default };
static int   IRQVectorDev[32] = { [0 ... 31] = -1 };

/**
 * \brief   This function calls the ISR of the highest priority IRQ
 *          It is called by the irq_handler routine when a CPU is interrupted by an IRQ
 *          Its aims is to find out which IRQ is now active to know which device needs the CPU.
 *          and to launch the right ISR of the right device instance by using IRQVector tables.
 */
void isrcall (void)
{
    int irq = icu_get_highest (cpuid());    // IRQ nb with the highest prio for the current cpu
    IRQVectorISR[irq] (IRQVectorDev[irq]);  // launch the ISR for the bound device
}

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
void arch_init (int tick)
{
    timer_init (0, tick);           // sets period of timer n'0 (thus for CPU n'0) and starts it
    icu_set_mask (0, 0);            // [CPU n'0].IRQ <-- ICU.PIN[0] <- Interrupt signal timer n'0
    IRQVectorISR [0] = timer_isr;   // tell the kernel which isr to exec for ICU.PIN n'0
    IRQVectorDev [0] = 0;           // device instance attached to ICU.PIN n'0

    for (int tty = 1; tty < NTTYS; tty++) { // TTY 0 is the console thus not used to get chars
        icu_set_mask (0, 10+tty);           // [CPU 0].IRQ <- ICU.PIN[10+tty] <- Int signal TTY tty
        IRQVectorISR [10+tty] = tty_isr;    // tell the kernel which isr is for ICU.PIN n'10+tty
        IRQVectorDev [10+tty] = tty;        // device instance attached to ICU.PIN n'10+tty
    }
}
