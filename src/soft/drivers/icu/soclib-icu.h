#ifndef _SOCLIB_ICU_H_
#define _SOCLIB_ICU_H_

#include <hal/arch/icu.h>

struct soclib_icu_regs_s {
    int state;          // state of all IRQ signals
    int mask;           // IRQ mask to chose what we need for this ICU
    int set;            // IRQ set   --> enable specific IRQs for this ICU
    int clear;          // IRQ clear --> disable specific IRQs for this ICU
    int highest;        // highest pritority IRQ number for this ICU
    int unused[3];      // 3 register addresses are not used
};

extern struct icu_ops_s soclib_icu_ops;

#endif