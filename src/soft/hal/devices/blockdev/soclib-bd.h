/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-03-30
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     hal/devices/blockdev/soclib-bd.h
  \author   Franck Wajsburt
  \brief    Soclib block device driver

\*------------------------------------------------------------------------------------------------*/

#ifndef _SOCLIB_BD_H_
#define _SOCLIB_BD_H_

#include <kernel/klibc.h>
#include <hal/devices/blockdev.h>

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
 * \brief   ISR of the soclib block device
 * \param   irq irq linked to the ISR 
 * \param   bd  device linked to the ISR
 * \return  nothing
 */
extern void soclib_bd_isr(unsigned irq, struct blockdev_s *bd);

/**
 * \brief See hal/device/blockdev.h for the function signature 
 * .blockdev_init     : initialize
 * .blockdev_read     : read the disk
 * .blockdev_write    : write the disk
 * .blockdev_setevent : define the callback function to be called at  the IRQ event
 */
extern struct blockdev_ops_s SoclibBDOps;

#endif
