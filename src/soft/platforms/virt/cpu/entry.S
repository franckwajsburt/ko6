//--------------------------------------------------------------------------------------------------
// Boot code
//--------------------------------------------------------------------------------------------------

.section .boot,"ax"                     // new section: .boot (see https://bit.ly/3gzKWob)
                                        // flags "ax": a -> allocated means section is in memory
                                        //             x -> section contains instructions

boot:                                   // qemu put pc at 0x80000000
    la      t0,     1 << 3
    csrc    x0,     mstatus,    t0      // disable interrupts (mstatus.MIE = 0)

    // mie.MTIE = 1, mie.MEIE = 1, mie.MSIE = 1
    // it will enable all interrupts when mstatus.MIE = 1
    la      t0,     1 << 3 | 1 << 7 | 1 << 11
    csrs    x0,     mie,        t0

    csrw    mtvec,  kentry              // set interrupt vector
    
    /* TODO: use pmp to protect kernel area */
    la      sp,     __kdata_end         // define stack ptr (first address after kdata region)
    call    kinit

.section .text

//--------------------------------------------------------------------------------------------------
// kernel entry for all causes,
//--------------------------------------------------------------------------------------------------

kentry:                                 // kernel entry
