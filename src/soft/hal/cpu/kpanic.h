#ifndef _CPU_KPANIC_H_
#define _CPU_KPANIC_H_

/**
 * \brief  prints all registers' value on TTY0 (must be in kernel mode) then stops program
 */
extern void kpanic (void);

#endif