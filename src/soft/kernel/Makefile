#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date      2022-07-03
# | / /(     )/ _ \   	\copyright 2021-2022 Sorbonne University
# |_\_\ x___x \___/                https://opensource.org/licenses/MIT
#
# \file     kernel/Makefile
# \author   Franck Wajsburt
# \brief    Makefile to produce the kernel executable: kernel.x
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

SOC	   ?= almo1-mips#				defaut SOC name
NTTYS  ?= 1#						default number of ttys
NCPUS  ?= 1#						default number of CPUS
VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()
BLDDIR	= ../../../build#			build directory
SRCDIR 	= ..
COMDIR	= $(SRCDIR)/common#			user & kernel common files directory
LIBDIR	= .#						library directory (where is ldscript)
HALDIR	= $(SRCDIR)/hal#			hardware abstraction layer (hal) files directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
OBJDIR	= $(BLDDIR)/obj#			object files directory
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory
LIBDIR  = ../../lib#				external libraries directory

# Sources
# --------------------------------------------------------------------------------------------------

SRC     = $(COMDIR)/debug_on.h $(COMDIR)/debug_off.h
SRC    += $(COMDIR)/syscalls.h ksyscalls.c
SRC    += $(COMDIR)/usermem.h $(COMDIR)/list.h
SRC    += $(COMDIR)/errno.h $(COMDIR)/errno.c $(COMDIR)/esc_code.h
SRC	   += $(COMDIR)/cstd.c $(COMDIR)/cstd.h
SRC    += kmemory.c kmemory.h
SRC    += klibc.c klibc.h
SRC    += kthread.c kthread.h
SRC    += kinit.c
SRC    += ksynchro.c ksynchro.h
LDSCRIPT= $(SOCDIR)/kernel.ld#		ldscript is hal dependent

# Targets
# --------------------------------------------------------------------------------------------------

SRCS    = $(filter %.S,$(SRC))
SRCC    = $(filter %.c,$(SRC))
OBJS    = $(subst .S,.o,$(foreach src,$(SRCS),$(notdir $(src))))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJS) $(OBJC))
OBJ 	+= $(OBJDIR)/libfdt.a

OBJDS   = $(addsuffix .s,$(OBJ))
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  the pdf file has the name of directory

# Tools
# --------------------------------------------------------------------------------------------------

include $(SRCDIR)/platforms/$(SOC)/$(SOC).mk

CFLAGS += -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I..#						directories where include files like <file.h> are located
CFLAGS += -I$(LIBDIR)/libfdt#		include external libraries (specifically libfdt.h)
CFLAGS += -DNTTYS=$(NTTYS)#			#define NTTYS with the number of ttys in the prototype
CFLAGS += -DNCPUS=$(NCPUS)#			#define NCPUS with the number of cpus in the prototype
CFLAGS += -DVERBOSE=$(VERBOSE)#		verbose if 1, can be toggled with #include <debug_{on,off}.h>

# Rules (here they are used such as simple shell scripts)
# --------------------------------------------------------------------------------------------------

.PHONY: help compil mkdir pdf clean depend

help:
	@echo ""
	@echo "Usage : make <compil|clean|pdf>"
	@echo ""
	@echo "        compil  : compile all sources"
	@echo "        clean   : clean up all compiled files"
	@echo "        pdf     : generate $(PDF) with all source files"
	@echo ""

compil: depend mkdir $(OBJ)

mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(OBJDIR) || (echo "- mkdir   $(OBJDIR)" ; mkdir $(OBJDIR))

pdf:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(PDFDIR) || (echo "- mkdir   $(PDFDIR)" ; mkdir $(PDFDIR))
	a2ps -T4 -2 -M A4 -A fill -o - -l100 Makefile $(SRC) 2> $(PDF).log |\
	ps2pdf -sPAPERSIZE=a4 - $(PDF);\
	echo "- create $(PDF)";\
	sed 's/^/  - /;s/\].*/\]/' $(PDF).log

clean:
	@echo "- clean up build files"
	-rm *~ *.dtb $(OBJ) $(OBJDS) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Rules with automatic variables to produce the executable
# --------------------------------------------------------------------------------------------------

$(OBJDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(COMDIR)/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s


# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCC) $(SRCS)
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/$(SOCDIR)/:/:' Makefile > Makefile.bak
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/$(COMDIR)/:/:' Makefile.bak > Makefile
	rm Makefile.bak

# --------------------------------------------------------------------------------------------------
# DEPENDENCIES

<<<<<<< HEAD
../../../build/obj/ksyscalls.o: ./klibc.h ../common/debug_off.h
../../../build/obj/ksyscalls.o: ../common/debug_on.h ../common/list.h
../../../build/obj/ksyscalls.o: ../hal/hcpu.h ../hal/almo1/hcpu_soc.h
../../../build/obj/ksyscalls.o: ../hal/harch.h ../common/usermem.h
../../../build/obj/ksyscalls.o: ./kmemory.h ./kthread.h ../common/esc_code.h
../../../build/obj/ksyscalls.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/ksyscalls.o: ./ksynchro.h
../../../build/obj/errno.o: ../common/errno.h
../../../build/obj/harch.o: ./klibc.h ../common/debug_off.h
../../../build/obj/harch.o: ../common/debug_on.h
../../../build/obj/harch.o: ../common/list.h ../hal/hcpu.h
../../../build/obj/harch.o: ../hal/almo1/hcpu_soc.h
../../../build/obj/harch.o: ../hal/harch.h ../common/usermem.h
../../../build/obj/harch.o: ./kmemory.h ./kthread.h
../../../build/obj/harch.o: ../common/esc_code.h
../../../build/obj/harch.o: ../common/errno.h
../../../build/obj/harch.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/hcpuc.o: ./klibc.h ../common/debug_off.h
../../../build/obj/hcpuc.o: ../common/debug_on.h
../../../build/obj/hcpuc.o: ../common/list.h ../hal/hcpu.h
../../../build/obj/hcpuc.o: ../hal/almo1/hcpu_soc.h
../../../build/obj/hcpuc.o: ../hal/harch.h ../common/usermem.h
../../../build/obj/hcpuc.o: ./kmemory.h ./kthread.h
../../../build/obj/hcpuc.o: ../common/esc_code.h
../../../build/obj/hcpuc.o: ../common/errno.h
../../../build/obj/hcpuc.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/kmemory.o: ./klibc.h ../common/debug_off.h
../../../build/obj/kmemory.o: ../common/debug_on.h ../common/list.h
../../../build/obj/kmemory.o: ../hal/hcpu.h ../hal/almo1/hcpu_soc.h
../../../build/obj/kmemory.o: ../hal/harch.h ../common/usermem.h ./kmemory.h
../../../build/obj/kmemory.o: ./kthread.h ../common/esc_code.h
../../../build/obj/kmemory.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/kmemory.o: ./ksynchro.h
../../../build/obj/klibc.o: ./klibc.h ../common/debug_off.h
../../../build/obj/klibc.o: ../common/debug_on.h ../common/list.h
../../../build/obj/klibc.o: ../hal/hcpu.h ../hal/almo1/hcpu_soc.h
../../../build/obj/klibc.o: ../hal/harch.h ../common/usermem.h ./kmemory.h
../../../build/obj/klibc.o: ./kthread.h ../common/esc_code.h
../../../build/obj/klibc.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/klibc.o: ./ksynchro.h
../../../build/obj/kthread.o: ./klibc.h ../common/debug_off.h
../../../build/obj/kthread.o: ../common/debug_on.h ../common/list.h
../../../build/obj/kthread.o: ../hal/hcpu.h ../hal/almo1/hcpu_soc.h
../../../build/obj/kthread.o: ../hal/harch.h ../common/usermem.h ./kmemory.h
../../../build/obj/kthread.o: ./kthread.h ../common/esc_code.h
../../../build/obj/kthread.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/kthread.o: ./ksynchro.h
../../../build/obj/kinit.o: ./klibc.h ../common/debug_off.h
../../../build/obj/kinit.o: ../common/debug_on.h ../common/list.h
../../../build/obj/kinit.o: ../hal/hcpu.h ../hal/almo1/hcpu_soc.h
../../../build/obj/kinit.o: ../hal/harch.h ../common/usermem.h ./kmemory.h
../../../build/obj/kinit.o: ./kthread.h ../common/esc_code.h
../../../build/obj/kinit.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/kinit.o: ./ksynchro.h
../../../build/obj/ksynchro.o: ./klibc.h ../common/debug_off.h
../../../build/obj/ksynchro.o: ../common/debug_on.h ../common/list.h
../../../build/obj/ksynchro.o: ../hal/hcpu.h ../hal/almo1/hcpu_soc.h
../../../build/obj/ksynchro.o: ../hal/harch.h ../common/usermem.h ./kmemory.h
../../../build/obj/ksynchro.o: ./kthread.h ../common/esc_code.h
../../../build/obj/ksynchro.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/ksynchro.o: ./ksynchro.h
../../../build/obj/hcpua.o: ../hal/hcpu.h
../../../build/obj/hcpua.o: ../hal/almo1/hcpu_soc.h
../../../build/obj/hcpua.o: ../common/syscalls.h
=======
../../../build/obj/ksyscalls.o: ../hal/arch/dma.h ../kernel/klibc.h
../../../build/obj/ksyscalls.o: ../common/debug_off.h ../common/debug_on.h
../../../build/obj/ksyscalls.o: ../common/cstd.h ../common/list.h
../../../build/obj/ksyscalls.o: ../common/esc_code.h ../common/errno.h
../../../build/obj/ksyscalls.o: ../common/syscalls.h ../common/usermem.h
../../../build/obj/ksyscalls.o: ../hal/cpu/atomic.h ../hal/cpu/cache.h
../../../build/obj/ksyscalls.o: ../hal/arch/platform.h ../hal/cpu/thread.h
../../../build/obj/ksyscalls.o: ../hal/cpu/irq.h ../hal/cpu/cpu.h
../../../build/obj/ksyscalls.o: ../hal/arch/chardev.h ../hal/arch/dev.h
../../../build/obj/ksyscalls.o: ../kernel/kmemory.h ../kernel/kthread.h
../../../build/obj/ksyscalls.o: ../kernel/ksynchro.h
../../../build/obj/errno.o: ../common/errno.h
../../../build/obj/cstd.o: ../common/cstd.h
../../../build/obj/kmemory.o: ../kernel/klibc.h ../common/debug_off.h
../../../build/obj/kmemory.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/kmemory.o: ../common/list.h ../common/esc_code.h
../../../build/obj/kmemory.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/kmemory.o: ../common/usermem.h ../hal/cpu/atomic.h
../../../build/obj/kmemory.o: ../hal/cpu/cache.h ../hal/arch/platform.h
../../../build/obj/kmemory.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/kmemory.o: ../hal/cpu/cpu.h ../hal/arch/chardev.h
../../../build/obj/kmemory.o: ../hal/arch/dev.h ../kernel/kmemory.h
../../../build/obj/kmemory.o: ../kernel/kthread.h ../kernel/ksynchro.h
../../../build/obj/klibc.o: ../kernel/klibc.h ../common/debug_off.h
../../../build/obj/klibc.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/klibc.o: ../common/list.h ../common/esc_code.h
../../../build/obj/klibc.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/klibc.o: ../common/usermem.h ../hal/cpu/atomic.h
../../../build/obj/klibc.o: ../hal/cpu/cache.h ../hal/arch/platform.h
../../../build/obj/klibc.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/klibc.o: ../hal/cpu/cpu.h ../hal/arch/chardev.h
../../../build/obj/klibc.o: ../hal/arch/dev.h ../kernel/kmemory.h
../../../build/obj/klibc.o: ../kernel/kthread.h ../kernel/ksynchro.h
../../../build/obj/kthread.o: ../kernel/klibc.h ../common/debug_off.h
../../../build/obj/kthread.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/kthread.o: ../common/list.h ../common/esc_code.h
../../../build/obj/kthread.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/kthread.o: ../common/usermem.h ../hal/cpu/atomic.h
../../../build/obj/kthread.o: ../hal/cpu/cache.h ../hal/arch/platform.h
../../../build/obj/kthread.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/kthread.o: ../hal/cpu/cpu.h ../hal/arch/chardev.h
../../../build/obj/kthread.o: ../hal/arch/dev.h ../kernel/kmemory.h
../../../build/obj/kthread.o: ../kernel/kthread.h ../kernel/ksynchro.h
../../../build/obj/kinit.o: ../kernel/klibc.h ../common/debug_off.h
../../../build/obj/kinit.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/kinit.o: ../common/list.h ../common/esc_code.h
../../../build/obj/kinit.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/kinit.o: ../common/usermem.h ../hal/cpu/atomic.h
../../../build/obj/kinit.o: ../hal/cpu/cache.h ../hal/arch/platform.h
../../../build/obj/kinit.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/kinit.o: ../hal/cpu/cpu.h ../hal/arch/chardev.h
../../../build/obj/kinit.o: ../hal/arch/dev.h ../kernel/kmemory.h
../../../build/obj/kinit.o: ../kernel/kthread.h ../kernel/ksynchro.h
../../../build/obj/ksynchro.o: ../kernel/klibc.h ../common/debug_off.h
../../../build/obj/ksynchro.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/ksynchro.o: ../common/list.h ../common/esc_code.h
../../../build/obj/ksynchro.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/ksynchro.o: ../common/usermem.h ../hal/cpu/atomic.h
../../../build/obj/ksynchro.o: ../hal/cpu/cache.h ../hal/arch/platform.h
../../../build/obj/ksynchro.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/ksynchro.o: ../hal/cpu/cpu.h ../hal/arch/chardev.h
../../../build/obj/ksynchro.o: ../hal/arch/dev.h ../kernel/kmemory.h
../../../build/obj/ksynchro.o: ../kernel/kthread.h ../kernel/ksynchro.h
>>>>>>> virt-riscv
