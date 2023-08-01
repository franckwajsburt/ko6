/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     common/user_mem.h
  \author   Franck Wajsburt
  \brief    User application memory description 

\*------------------------------------------------------------------------------------------------*/

#ifndef _USERMEM_H_
#define _USERMEM_H_

/** 
 * \brief few data about user memory usage, stucture placed at the bottom of the .data region
 *
 * +-------------+ <- ustack_begin  points to the first address ABOVE the first user stack
 * | user  stack |                  The last word of each stack contains also a MAGIC number 
 * | thread main |                  in order to be able to know if the stack has overflowed
 * +-------------+                  This check must be done sometimes because                
 * | user  stack |                  there is not MMU to check it at each access              
 * |  thread 1   |                    
 * +-------------+ <- ustack_end    points to the last cell of the last user stack
 * |/////////////|                  then ustack_end points inside of the last stack
 * |/////////////|
 * +-------------+ <- uheap_end     points to the very first address ABOVE the user heap
 * |             |                  uheap_end is always inferior or equal to stack_end
 * |  user heap  |                  uheap_end is moved with SYSCALL_BRK
 * |             |              
 * +-------------+ <- uheap_begin   points to the very first address of the user heap
 * 
 */
typedef struct usermem_s {
    int * ustack_beg;       ///< highest address of the user stack segment
    int * ustack_end;       ///< lowest address of the user stack segment
    int * uheap_end;        ///< highest address of the user heap segment (also named brk)
    int * uheap_beg;        ///< lowest highest address of the user heap segment
} usermem_t;

extern usermem_t _user_mem; // This structure is in user memory space (in this version)

#define PAGE_SIZE       4096            // page size, can't be changed
#define USTACK_SIZE     (16*PAGE_SIZE)  // thread stack size (all thread have the same size)
#define MAGIC_STACK     0xDEADF00D      // used to tag user stack (to check corruption)
#define MAGIC_HEAP      0x5A            // used to tag user heap block (to check corruption)



#endif//_USERMEM_H_
