# Adding syscalls to ko6

Add you're syscall in `/src/soft/common/syscals.h` with a define.

```c
#define SYSCALL_NEW 27
```

You'll need to create the function that will be executed whenever you call the syscall. You can pass up to 4 arguments.

You'll need to define this in the folder `/src/soft/kernel/`. Do this in a propper creating a header file and a source file.

For the header file `knewsys.h`:
```c
#ifndef _KNEWSYS_H_
#define _KNEWSYS_H_

#include <kernel/klibc.h>

void sys_newsys(int a0, int a1, int a2, int a3);

#endif
```

The source file:
```c
#include <knewsys.h>

void sys_knewsys(int a0, int a1, int a2, int a3) {
    kprintf("hello from sys_kshell!\n");
    kprintf("a0: %d, a1: %d, a2: %d, a3: %d\n", a0, a1, a2, a3);
}
```

After, this, add both header file and source file to the SRC variable in the Makefile at `/src/soft/kernel/`:
```makefile
SRC    += knewsys.c knewsys.h
```

> the next part may change

Now, you need to include the header file in the `/src/soft/kernel/klib.c` to make your new syscall available with the other kernel API functions.

```c
#include <kernel/knewsys.h>
```

You'll also need to add the syscall function to the Syscall Vector in the `/src/soft/kernel/ksyscalls.c`:

```c
/* ... */
void *SyscallVector[] = {
    [0 ... SYSCALL_NR - 1   ] = unknown_syscall,   /* default function */
    /* ... */
    [SYSCALL_KSHELL         ] = sys_kshell,
    [SYSCALL_NEW            ] = sys_knewsys,
};
/* ... */
```

Now, you need to specify the object file it's going to be generated in `/src/soft/hal/soc/almo1-mips/Makefile`. 
```makefile
# ...
OBJ 	+= $(OBJDIR)/kdev.o $(OBJDIR)/kirq.o
OBJ		+= $(OBJDIR)/knewsys.o
OBJ 	+= $(OBJDIR)/libfdt.a
OBJDS   = $(addsuffix .s,$(OBJ))
# ...
```

Finally, you have to write a program that uses you're brand new syscall in order to test it.

## Bonus

Take a look to the files in `/src/soft/hal/<arch>/cpu_user.S`. This is the assembly code that corresponds to the calling a syscall. For example, for mips you'll have:

```S
.globl syscall_fct
syscall_fct: 
    lw  $2,16($29)
    syscall
    jr  $31

```

