/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-04-14
  | / /(     )/ _ \     \copyright  2025 Sorbonne University
  |_\_\ x___x \___/     \license    https://opensource.org/licenses/MIT

  \file     hal/devices/blockdev/soclib-bd.h
  \author   Franck Wajsburt
  \brief    Soclib block device driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/blockdev/soclib-bd.h>
#include <kernel/kdev.h>
#include <hal/cpu/atomic.h>

/**
 * Soclib device registers, commands, and errors
 */

enum bdops_e {
    BD_NOOP,  			    ///< default value, if the BD does nothing
    BD_READ, 			    ///< BD starts to move data FROM the disk
    BD_WRITE 			    ///< BD starts to move data TO the disk
};

enum bdstatus_e {
    BD_IDLE, 			    ///< default value, if the BD does nothing
    BD_BUSY, 			    ///< BD is already doing an operation (if no IRQ wanted)
    BD_READ_SUCCESS, 	    ///< BD has successfully completed and COUNT == 0
    BD_WRITE_SUCCESS, 	    ///< BD has successfully completed and COUNT == 0
    BD_READ_ERROR, 		    ///< BD could not finish, COUNT != 0
    BD_WRITE_ERROR, 	    ///< BD could not finish, COUNT != 0
    BD_ERROR 			    ///< BD has a fatal error (e.g. lost disk, impossible here)
};

struct soclib_bd_regs_s {
    void * buffer; 	    	///< (W) buffer address in memory (aligned on a PHYSICAL logic block)
    unsigned pba; 		    ///< (W) PHYSICAL bloc address in the disk (not a LOGICAL bloc address)
    unsigned count; 		///< (RW) size in PHYSICAL blocks to move
    enum bdops_e op; 	    ///< (W) transaction direction (read from disk or write from disk)   
    enum bdstatus_e status; ///< (R) block device status at the end of operation (acknowledges irq)
    unsigned irq_enable;    ///< (RW) enable/disable the irq line
    unsigned size; 			///< (R) disk size in PHYSICAL blocks
    unsigned block_size;	///< (R) size of a PHYSICAL block in bytes (typically 512)
};

/**
 * \brief   Initialize the Soclib block device
 * \param   bdev       The block device 
 * \param   base       The base address of the device
 * \param   block_size size of a LOGICAL block chosen by ko6
 * \return  nothing
 */
static void soclib_bd_init(struct blockdev_s *bdev, unsigned base, unsigned block_size)
{
    bdev->base    = base;
    bdev->ops     = &SoclibBDOps;
    volatile struct soclib_bd_regs_s *regs = 
        (struct soclib_bd_regs_s *) bdev->base;
    bdev->block_size = block_size;              // LOGICAL block size (e.g. 4096)
    bdev->ppb = block_size / regs->block_size;  // nb of physical blocks per logical blocks
    bdev->blocks  = regs->size / bdev->ppb;     // disk size in LOGICAL blocks
}

/**
 * \brief   read logical blocks
 * \param   bdev    the blockdev device
 * \param   lba     the logical block address where the data is written
 * \param   buf     the buffer where to read the data to be written to the block device
 * \param   count   the number of logical block to move
 * \return  0 on success, -EINVAL if invalid arguments
 */
#include <common/debug_on.h>

static int soclib_bd_read(struct blockdev_s *bdev, unsigned lba, void *buf, unsigned count)
{
    if (!buf || !bdev || ((lba+count) >= bdev->blocks)) 
        return errno = -EINVAL; // wrong parameters
VAR(%p\n,bdev);
VAR(%p\n,lba);
VAR(%p\n,buf);
VAR(%p\n,count);
    volatile struct soclib_bd_regs_s *regs = 
        (struct soclib_bd_regs_s *) bdev->base;
    regs->buffer = buf;                         // destination address in memory
    regs->pba = lba * bdev->ppb;                // source address in disk (physical block address)
    regs->count = count * bdev->ppb;            // number of physical blocks to move
    regs->op = BD_READ;                         // at last command, the transfer is starting
    dcache_buf_invalidate(buf, count);          // if there are cached lines dst buffer, forget them
VAR(%p\n,regs);
VAR(%p\n,bdev->ppb);
// FIXME this part is temporaty
    while (regs->status == BD_BUSY) delay(100);
    if (regs->status != BD_READ_SUCCESS) 
        return errno = -EIO; 
// ---------------------------
    return 0;
}

/**
 * \brief   write logical blocks
 * \param   bdev    the blockdev device
 * \param   lba     the logical block address where the data is read
 * \param   buf     the buffer where to write the data to be read from the block device
 * \param   count   the number of logical blocks to move
 * \return  0 on success, -EINVAL if invalid arguments
 */
static int soclib_bd_write(struct blockdev_s *bdev, unsigned lba, void *buf, unsigned count)
{
    if (!buf || !bdev || ((lba+count) >= bdev->blocks)) return -EINVAL; // wrong parameters
    volatile struct soclib_bd_regs_s *regs = 
        (struct soclib_bd_regs_s *) bdev->base;
    regs->buffer = buf;                         // source address in memory
    regs->pba = lba * bdev->ppb;                // destination address in disk (physical block addr)
    regs->count = count * bdev->ppb;            // number of physical blocks to move
    regs->op = BD_WRITE;                        // at last command, the transfer is starting
// FIXME this part is temporaty
    while (regs->count) delay(100);
    if (regs->status != BD_WRITE_SUCCESS) return -EIO; 
// ---------------------------
    return 0;
}

/**
 * \brief   Set the event that will triggered by a soclib block device interrupt
 * \param   bdev the block device
 * \param   f    the function corresponding to the event
 * \param   arg  argument that will be passed to the function
 * \return  nothing
 */
static void soclib_bd_set_event(struct blockdev_s *bdev, void(*f)(void *arg, int status), void *arg)
{
    bdev->event.f = f;
    bdev->event.arg = arg;
}

struct blockdev_ops_s SoclibBDOps = {
    .blockdev_init = soclib_bd_init,
    .blockdev_read = soclib_bd_read,
    .blockdev_write = soclib_bd_write,
    .blockdev_set_event = soclib_bd_set_event
};

void soclib_bd_isr(unsigned irq, struct blockdev_s *bdev)
{
    struct soclib_bd_regs_s *regs =
        (struct soclib_bd_regs_s *) bdev->base;
    int status = regs->status;          // IRQ acknoledgement to lower the interrupt signal

    if (bdev->event.f)
        bdev->event.f (bdev->event.arg, status);
}

/*------------------------------------------------------------------------------------------------*\
   Editor config (vim/emacs): tabs are 4 spaces, max line length is 100 characters
   vim: set ts=4 sw=4 sts=4 et tw=100:
   -*- mode: c; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
\*------------------------------------------------------------------------------------------------*/
