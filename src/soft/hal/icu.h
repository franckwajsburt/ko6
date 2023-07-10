/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-10
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/icu.h
  \author   Franck Wajsburt, Nolan Bled
  \brief    Generic ICU functions prototypes

\*------------------------------------------------------------------------------------------------*/

#ifndef _HAL_ICU_H_
#define _HAL_ICU_H_

struct icu_s;

struct icu_ops_s {
    void        (*icu_init)(struct icu_s *icu, unsigned address);
    unsigned    (*icu_get_highest)(struct icu_s *icu);
    void        (*icu_set_priority)(struct icu_s *icu, unsigned irq, unsigned pri);
    void        (*icu_acknowledge)(struct icu_s *icu, unsigned irq);
    void        (*icu_mask)(struct icu_s *icu, unsigned irq);
    void        (*icu_unmask)(struct icu_s *icu, unsigned irq);
};

struct icu_s {
    unsigned address;
    struct icu_ops_s *ops;
};

/* ICU must be defined once somewhere */
extern struct icu_s icu;

#endif