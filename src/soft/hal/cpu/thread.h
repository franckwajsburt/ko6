/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-24
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/cpu/thread.h
  \author   Franck Wajsburt
  \brief    Generic CPU functions prototypes

\*------------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------------------
// Thread management
//--------------------------------------------------------------------------------------------------

/**
 * \brief   Initialize a user thread context at the very beginning
 * \param   context       thread_context table 
 * \param   bootstrap     function without any argument, used just after the 1st thread_context_load
 * \param   stack_pointer top stack pointer at the very beginning
 * \return  side effect on the given thread context table
 */
extern void thread_context_init (int context[], void * bootstrap, void * stack_pointer);

/**
 * \brief   Initialize a kernel thread context at the very beginning
 * \param   context       thread_context table 
 * \param   bootstrap     function without any argument, used just after the 1st thread_context_load
 * \param   stack_pointer top stack pointer at the very beginning
 * \return  side effect on the given thread context table
 */
extern void kthread_context_init (int context[], void * bootstrap, void * stack_pointer);

/**
 * \brief   kernel saves the given thread registers in the context table
 * \param   context thread_context table of the thread to save
 * \return  it returns two values depending on how this function has been called.
 *          First case, the simplest, we call thread_save () then it returns 1 (true).
 *          Secondly, we call thread_restore() (see bellow) that causes the restoration
 *          of all registers recorded by thread_save(), including $31,
 *          thus when we return from thread_load(), in reality, we return from thread_save,
 *          but with 0 (false) as return value.
 */
extern int thread_context_save (int context[]);

/**
 * \brief   kernel load the given thread registers from the context table
 * \param   context thread_context table  of the thread to load (or to restore)
 * \return  returns alway 0 (false)
 */
extern int thread_context_load (int context[]);

/**
 * \brief   The kernel starts any thread for the very first time with this function.
 *          This function is called by the static function thread_bootstrap() (in kthread.c)
 *          which is the real beginning of a thread, and thread_bootstrap() is called by
 *          the first call of thread_load() for each thread (see comment of thread_bootstrat in
 *          kthread.c for more details)
 * \param   fun     ptr to the function of the thread, type:  void*(*fun)(void*) but cast to int
 * \param   arg     the argument given to fun(), type:   void*  but cast to int
 * \param   start   ptr to the user function which calls fun(arg) for real, type is cast to int
 *                  In fact, start has two possible types.
 *                  If it is the main thread then it is :
 *                      void (*start) (void)
 *                  but if it is a standard thread then it is :
 *                      void (*start) (void *(*fun) (void *), void *arg)
 * \return  never returns
 */
extern int thread_launch (int fun, int arg, int start);
