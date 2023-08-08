/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/icu.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Generic ICU functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_ICU_H_
#define _HAL_ICU_H_

struct icu_s;

/** 
 * \brief Functions prototypes of the ICU device, they should be implemented
 *        by a device driver. They serve as an interface between the kernel and
 *        the driver
 */
struct icu_ops_s {
    /**
     * \brief   Generic function to initialize the ICU device
     * \param   address the base address of the memory-mapped registers
     */
    void        (*icu_init)(struct icu_s *icu, unsigned address);

    /**
     * \brief   Generic function that fetch the highest priority IRQ from the ICU device
     * \param   icu the icu device
     * \return  highest priority IRQ
     */
    unsigned    (*icu_get_highest)(struct icu_s *icu);

    /**
     * \brief   Generic function that sets the priority of a given IRQ
     * \param   icu the icu device
     * \param   irq the irq
     * \param   pri the new priority of the IRQ
     */
    void        (*icu_set_priority)(struct icu_s *icu, unsigned irq, unsigned pri);

    /**
     * \brief   Generic function that should aknowledge an IRQ
     * \param   icu the icu device
     * \param   irq the irq to acknowledge
     */
    void        (*icu_acknowledge)(struct icu_s *icu, unsigned irq);

    /**
     * \brief   Generic function that mask (disable) an IRQ
     * \param   icu the icu device
     * \param   irq the irq to mask
     */
    void        (*icu_mask)(struct icu_s *icu, unsigned irq);

    /**
     * \brief   Generic function that unmask (enable) an IRQ
     * \param   icu the icu device
     * \param   irq the irq to unmask
     */   
    void        (*icu_unmask)(struct icu_s *icu, unsigned irq);
};

/** \brief ICU driver informations */
struct icu_s {
    unsigned address;           //< ICU's address
    struct icu_ops_s *ops;      //< driver-specific operations
};
#define icu_alloc() (struct icu_s*) (dev_alloc(ICU_DEV, sizeof(struct icu_s))->data)
#define icu_get(no) (struct icu_s*) (dev_get(ICU_DEV, no)->data)
#define icu_count() (dev_next_no(ICU_DEV) - 1)

#endif