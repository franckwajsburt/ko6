/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/irq.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Generic ICU functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_ICU_H_
#define _HAL_ICU_H_

struct irq_s;

/** 
 * \brief Functions prototypes of the ICU device, they should be implemented by a device driver. 
 *        They serve as an interface between the kernel and the driver
 */
struct irq_ops_s {
    /**
     * \brief   Generic function to initialize the ICU device
     * \param   base the base address of the memory-mapped registers
     */
    void        (*irq_init)(struct irq_s *irq, unsigned base);

    /**
     * \brief   Generic function that fetch the highest priority IRQ from the ICU device
     * \param   irq the irq device
     * \return  highest priority IRQ
     */
    unsigned    (*irq_get_highest)(struct irq_s *irq);

    /**
     * \brief   Generic function that sets the priority of a given IRQ
     * \param   irq the irq device
     * \param   irq the irq
     * \param   pri the new priority of the IRQ
     */
    void        (*irq_set_priority)(struct irq_s *irq, unsigned irq, unsigned pri);

    /**
     * \brief   Generic function that should aknowledge an IRQ
     * \param   irq the irq device
     * \param   irq the irq to acknowledge
     */
    void        (*irq_acknowledge)(struct irq_s *irq, unsigned irq);

    /**
     * \brief   Generic function that mask (disable) an IRQ
     * \param   irq the irq device
     * \param   irq the irq to mask
     */
    void        (*irq_mask)(struct irq_s *irq, unsigned irq);

    /**
     * \brief   Generic function that unmask (enable) an IRQ
     * \param   irq the irq device
     * \param   irq the irq to unmask
     */   
    void        (*irq_unmask)(struct irq_s *irq, unsigned irq);
};

/** \brief ICU driver informations */
struct irq_s {
    unsigned base;              //< ICU's base address
    struct irq_ops_s *ops;      //< driver-specific operations
};
#define irq_alloc() (struct irq_s*) (dev_alloc(ICU_DEV, sizeof(struct irq_s))->data)
#define irq_get(no) (struct irq_s*) (dev_get(ICU_DEV, no)->data)
#define irq_count() (dev_next_no(ICU_DEV) - 1)

#endif
