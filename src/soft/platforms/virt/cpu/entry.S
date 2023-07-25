//--------------------------------------------------------------------------------------------------
// Boot code
//--------------------------------------------------------------------------------------------------

.section .boot,"ax"                     // new section: .boot (see https://bit.ly/3gzKWob)
                                        // flags "ax": a -> allocated means section is in memory
                                        //             x -> section contains instructions

boot:                                   // qemu put pc at 0x80000000
    la      t0,     8
    csrrc   x0,     mstatus,    t0      // disable interrupts
    la      mtvec,  kentry              // set interrupt vector
    la      sp,     __kdata_end         // define stack ptr (first address after kdata region)
    call    kinit