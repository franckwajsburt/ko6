/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-21
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/icu.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Generic ICU functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_ICU_H_
#define _HAL_ICU_H_

struct icu_ops_s;

/** \brief ICU driver informations */
typedef struct icu_s {
    unsigned base;              ///< ICU's base address
    struct icu_ops_s *ops;      ///< driver-specific operations
} icu_t;

/** 
 * \brief Functions prototypes of the ICU device, they should be implemented by a device driver. 
 *        They serve as an interface between the kernel and the driver.
 */
struct icu_ops_s {
    /**
     * \brief   Generic function to initialize the ICU device
     * \param   base the base address of the memory-mapped registers
     * \note    almo1-mips : soclib_icu_init
     */
    void        (*icu_init)(icu_t *icu, unsigned base);

    /**
     * \brief   Generic function that fetch the highest priority IRQ from the ICU device
     * \param   icu the icu device
     * \return  highest priority IRQ
     * \note    almo1-mips : soclib_icu_get_highest
     */
    unsigned    (*icu_get_highest)(icu_t *icu);

    /**
     * \brief   Generic function that sets the priority of a given IRQ
     * \param   icu the icu device
     * \param   irq the irq
     * \param   pri the new priority of the IRQ
     * \note    almo1-mips : soclib_icu_set_priority
     */
    void        (*icu_set_priority)(icu_t *icu, unsigned irq, unsigned pri);

    /**
     * \brief   Generic function that should aknowledge an IRQ
     * \param   icu the icu device
     * \param   irq the irq to acknowledge
     * \note    almo1-mips : soclib_icu_acknowledge
     */
    void        (*icu_acknowledge)(icu_t *icu, unsigned irq);

    /**
     * \brief   Generic function that mask (disable) an IRQ
     * \param   icu the icu device
     * \param   irq the irq to mask
     * \note    almo1-mips : soclib_icu_mask
     */
    void        (*icu_mask)(icu_t *icu, unsigned irq);

    /**
     * \brief   Generic function that unmask (enable) an IRQ
     * \param   icu the icu device
     * \param   irq the irq to unmask
     * \note    almo1-mips : soclib_icu_unmask
     */   
    void        (*icu_unmask)(icu_t *icu, unsigned irq);
};

//--------------------------------------------------------------------------------------------------
// icu_alloc() is used once by soc_init/soc_icu_init to add a new device in the device tree
// icu_count() returns the number of icus in the current SoC
// icu_get(no) returns the icu device structure from its instance number
//--------------------------------------------------------------------------------------------------

#define icu_alloc() (icu_t *)(dev_alloc(ICU_DEV, sizeof(icu_t))->data)
#define icu_count() (dev_next_no(ICU_DEV) - 1)
#define icu_get(no) (icu_t *)(dev_get(ICU_DEV, no)->data)

#endif

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
