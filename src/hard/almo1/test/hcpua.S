/*------------------------------------------------------------------------------------------------*\
   _     ___    __  
  | |__ /'v'\  / /      \date        2023-01-30
  | / /(     )/ _ \     \copyright   2021-2023 Sorbonne University 
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT
  
\*------------------------------------------------------------------------------------------------*/

#ifndef NTTYS
#define NTTYS 4
#endif

.text                                   // section where all code are put

boot:

    la      $5, __tty_regs_map          // $5 <- address of tty's registers map (cf. ldscript)
    li      $9, NTTYS                   // $9 <- number of ttys
    li      $10, '0'                    // $10 <- ascii code of '0'

printall_mess:

    la      $4, hello                   // $4 <- address of string: "hello..."
    sb      $10, ($5)                   // send the current char to the tty

print_mess:

    lb      $8, ($4)                    // get current char
    sb      $8, ($5)                    // send the current char to the tty
    addiu   $4, $4, 1                   // point to the next char
    bnez    $8, print_mess              // check that it is not null, if yes it must be printed
    addiu   $5, $5, 0x10                // $5 jump to the next TTY
    addiu   $10,$10, 1                  // $10 <- next char to write
    addiu   $9, $9, -1                  // decrement the counter of the loop
    bnez    $9, printall_mess           // while there is another TTY to write

print_image:

    la      $4, __bd_regs_map           // $4 <- address of disk's registers
    la      $5, __fb_frame              // $5 <- address of frame buffer
    sw      $5, 0($4)                   // set buffer address for the disk 
    li      $5, 0                      // $5 <- first bloc number to read on disk
    sw      $5, 4($4)                   // set lba
    li      $5, 32                      // $5 <- number of blocs to read
    sw      $5, 8($4)                   // set count
    li      $5, 1                       // $5 <- read command
    sw      $5, 12($4)                  // set command and start

    li      $6, 30000
 l0:addiu   $6, $6, -1
    bne     $6, $0, l0

    la      $4, __bd_regs_map           // $4 <- address of disk's registers
    la      $5, __fb_frame              // $5 <- address of frame buffer
    sw      $5, 0($4)                   // set buffer address for the disk 
    li      $5, 2*32                      // $5 <- first bloc number to read on disk
    sw      $5, 4($4)                   // set lba
    li      $5, 32                      // $5 <- number of blocs to read
    sw      $5, 8($4)                   // set count
    li      $5, 1                       // $5 <- read command
    sw      $5, 12($4)                  // set command and start
    
    li      $6, 30000
 l1:addiu   $6, $6, -1
    bne     $6, $0, l1

    la      $4, __bd_regs_map           // $4 <- address of disk's registers
    la      $5, __fb_frame              // $5 <- address of frame buffer
    sw      $5, 0($4)                   // set buffer address for the disk 
    li      $5, 2*32                      // $5 <- first bloc number to read on disk
    sw      $5, 4($4)                   // set lba
    li      $5, 32                      // $5 <- number of blocs to read
    sw      $5, 8($4)                   // set count
    li      $5, 1                       // $5 <- read command
    sw      $5, 12($4)                  // set command and start
    
    li      $6, 30000
 l2:addiu   $6, $6, -1
    bne     $6, $0, l2

    la      $4, __bd_regs_map           // $4 <- address of disk's registers
    la      $5, __fb_frame              // $5 <- address of frame buffer
    sw      $5, 0($4)                   // set buffer address for the disk 
    li      $5, 3*32                      // $5 <- first bloc number to read on disk
    sw      $5, 4($4)                   // set lba
    li      $5, 32                      // $5 <- number of blocs to read
    sw      $5, 8($4)                   // set count
    li      $5, 1                       // $5 <- read command
    sw      $5, 12($4)                  // set command and start
    
    li      $6, 30000
 l3:addiu   $6, $6, -1
    bne     $6, $0, l3

    la      $4, __bd_regs_map           // $4 <- address of disk's registers
    la      $5, __fb_frame              // $5 <- address of frame buffer
    sw      $5, 0($4)                   // set buffer address for the disk 
    li      $5, 4*32                      // $5 <- first bloc number to read on disk
    sw      $5, 4($4)                   // set lba
    li      $5, 32                      // $5 <- number of blocs to read
    sw      $5, 8($4)                   // set count
    li      $5, 1                       // $5 <- read command
    sw      $5, 12($4)                  // set command and start

    li      $6, 30000
 l4:addiu   $6, $6, -1
    bne     $6, $0, l4

    la      $4, __bd_regs_map           // $4 <- address of disk's registers
    la      $5, __fb_frame              // $5 <- address of frame buffer
    sw      $5, 0($4)                   // set buffer address for the disk 
    li      $5, 5*32                      // $5 <- first bloc number to read on disk
    sw      $5, 4($4)                   // set lba
    li      $5, 32                      // $5 <- number of blocs to read
    sw      $5, 8($4)                   // set count
    li      $5, 1                       // $5 <- read command
    sw      $5, 12($4)                  // set command and start
    
    li      $6, 30000
 l5:addiu   $6, $6, -1
    bne     $6, $0, l5

    la      $4, __bd_regs_map           // $4 <- address of disk's registers
    la      $5, __fb_frame              // $5 <- address of frame buffer
    sw      $5, 0($4)                   // set buffer address for the disk 
    li      $5, 6*32                      // $5 <- first bloc number to read on disk
    sw      $5, 4($4)                   // set lba
    li      $5, 32                      // $5 <- number of blocs to read
    sw      $5, 8($4)                   // set count
    li      $5, 1                       // $5 <- read command
    sw      $5, 12($4)                  // set command and start

    li      $6, 30000
 l6:addiu   $6, $6, -1
    bne     $6, $0, l6

    la      $4, __bd_regs_map           // $4 <- address of disk's registers
    la      $5, __fb_frame              // $5 <- address of frame buffer
    sw      $5, 0($4)                   // set buffer address for the disk 
    li      $5, 7*32                      // $5 <- first bloc number to read on disk
    sw      $5, 4($4)                   // set lba
    li      $5, 32                      // $5 <- number of blocs to read
    sw      $5, 8($4)                   // set count
    li      $5, 1                       // $5 <- read command
    sw      $5, 12($4)                  // set command and start
    
    li      $6, 30000
 l7:addiu   $6, $6, -1
    bne     $6, $0, l7

dead:                                   // infinite loop
    j       print_image

hello:
    .asciiz " --> Hello World!\n"       // string to print (put for now in .text section)
