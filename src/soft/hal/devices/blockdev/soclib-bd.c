/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-28
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/blockdev/soclib-bd.h
  \author   Franck Wajsburt
  \brief    Soclib block device driver

\*------------------------------------------------------------------------------------------------*/

#include <hal/devices/blockdev/soclib-bd.h>

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
 * \param   buf     the buffer where to read the data to be written to the block device
 * \param   lba     the logical block address where the data is written
 * \param   count   the number of logical block to move
 * \return  number of blocks actually read from the disk
 */
static int soclib_bd_read(struct blockdev_s *bdev, void *buf, unsigned lba, unsigned count)
{
    volatile struct soclib_bd_regs_s *regs = 
        (struct soclib_bd_regs_s *) bdev->base;
    regs->buffer = buf;                         // destination address in memory
    regs->pba = lba * bdev->ppb;                // source address in disk (physical block address)
    regs->count = count * bdev->ppb;            // number of physical blocks to move
    regs->op = BD_READ;                         // at last command, the transfer is starting
    dcache_buf_invalidate(buf, count);          // if there are cached lines dst buffer, forget them
    return count;
}

/**
 * \brief   write logical blocks
 * \param   bdev    the blockdev device
 * \param   buf     the buffer where to write the data to be read from the block device
 * \param   lba     the logical block address where the data is read
 * \param   count   the number of logical blocks to move
 * \return  number of blocks actually written to the disk
 */
static int soclib_bd_write(struct blockdev_s *bdev, void *buf, unsigned lba, unsigned count)
{
    volatile struct soclib_bd_regs_s *regs = 
        (struct soclib_bd_regs_s *) bdev->base;
    regs->buffer = buf;                         // source address in memory
    regs->pba = lba * bdev->ppb;                // destination address in disk (physical block addr)
    regs->count = count * bdev->ppb;            // number of physical blocks to move
    regs->op = BD_WRITE;                        // at last command, the transfer is starting
    return count;
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
