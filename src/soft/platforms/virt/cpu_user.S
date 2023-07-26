/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-07-26
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     platform/virt/cpu_user.S
  \author   Nolan Bled
  \brief    CPU specific user functions 
            - This file contains cpu-dependant functions used by user applications. 
            - This functions could be static inline functions because or even macros,
              instead this is real code in .h file that is not very clean, 
              but I don't want theses functions inlined in order to see them in the trace execution
            - system calls 
            - FIXME spinlock or atomic add should be appeared.

\*------------------------------------------------------------------------------------------------*/

.globl syscall_fct      # it is an external function, it must be declared globl
syscall_fct:            # syscall_fct function label
    ecall
    ret

.globl sbrk_s
sbrk_s:
    sw  a0, 0(sp)
    ret
