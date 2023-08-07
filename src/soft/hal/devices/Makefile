#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date      2022-07-03
# | / /(     )/ _ \   	\copyright 2021-2022 Sorbonne University
# |_\_\ x___x \___/                https://opensource.org/licenses/MIT
#
# \file     drivers/Makefile
# \author   Nolan Bled
# \brief    Makefile to produce drivers used
# TODO: Update the driver build system to build only the necessary drivers
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

SOC 	?= almo1-mips
VERBOSE	?= 1

SRCDIR 	= ..#						directory of all sources
BLDDIR	= ../../../build#			build directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
HALDIR 	= ../hal#					hal files directory
COMDIR 	= ../common#				common files directory
OBJDIR	= $(BLDDIR)/obj#			object files directory
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory

# Sources
# --------------------------------------------------------------------------------------------------

SRC	=	dma/soclib-dma.c
SRC +=	icu/soclib-icu.c icu/plic.c
SRC += 	timer/soclib-timer.c timer/clint-timer.c
SRC += 	chardev/soclib-tty.c chardev/ns16550.c

# Targets
# --------------------------------------------------------------------------------------------------

SRCS    = $(filter %.S,$(SRC))
SRCC    = $(filter %.c,$(SRC))
OBJS    = $(subst .S,.o,$(foreach src,$(SRCS),$(notdir $(src))))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJS) $(OBJC))
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
CFLAGS += -I$(HALDIR)#				directories where include files like <file.h> are located
CFLAGS += -I.. -I../kernel
CFLAGS += -I../common
CFLAGS += -DVERBOSE=$(VERBOSE)#		verbose if 1, can be toggled with #include <debug_{on,off}.h>

# Rules (here they are used such as simple shell scripts)
# --------------------------------------------------------------------------------------------------

.PHONY: help compil mkdir pdf clean depend

help:
	@echo ""
	@echo "Usage : make <compil|clean|pdf>"
	@echo ""
	@echo "        compil  : compile all sources"
	@echo "        clean   : clean all compiled files"
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
	@echo "- clean $(CURDIR) and related build files"
	-rm *~ $(OBJ) $(OBJDS) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Rules with automatic variables to produce the executable
# --------------------------------------------------------------------------------------------------

$(OBJDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : dma/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : icu/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : timer/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : chardev/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCC) $(SRCS)
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/dma/:/:' Makefile |\
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/icu/:/:' |\
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/timer/:/:' |\
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/chardev/:/:' > Makefile.bak
	mv Makefile.bak Makefile

# --------------------------------------------------------------------------------------------------
# DEPENDENCIES

../../../build/obj/soclib-dma.o: ../drivers/dma/soclib-dma.h
../../../build/obj/soclib-dma.o: ../kernel/klibc.h ../common/debug_off.h
../../../build/obj/soclib-dma.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/soclib-dma.o: ../common/list.h ../common/esc_code.h
../../../build/obj/soclib-dma.o: ../common/errno.h ../common/syscalls.h
../../../build/obj/soclib-dma.o: ../common/usermem.h ../hal/cpu/atomic.h
../../../build/obj/soclib-dma.o: ../hal/cpu/cache.h
../../../build/obj/soclib-dma.o: ../hal/arch/platform.h
../../../build/obj/soclib-dma.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/soclib-dma.o: ../hal/cpu/cpu.h ../hal/arch/chardev.h
../../../build/obj/soclib-dma.o: ../kernel/kdev.h ../kernel/kmemory.h
../../../build/obj/soclib-dma.o: ../kernel/kthread.h ../kernel/ksynchro.h
../../../build/obj/soclib-dma.o: ../hal/arch/dma.h
../../../build/obj/soclib-icu.o: ../drivers/icu/soclib-icu.h
../../../build/obj/soclib-icu.o: ../hal/arch/icu.h
../../../build/obj/plic.o: ../drivers/icu/plic.h ../kernel/klibc.h
../../../build/obj/plic.o: ../common/debug_off.h ../common/debug_on.h
../../../build/obj/plic.o: ../common/cstd.h ../common/list.h
../../../build/obj/plic.o: ../common/esc_code.h ../common/errno.h
../../../build/obj/plic.o: ../common/syscalls.h ../common/usermem.h
../../../build/obj/plic.o: ../hal/cpu/atomic.h ../hal/cpu/cache.h
../../../build/obj/plic.o: ../hal/arch/platform.h ../hal/cpu/thread.h
../../../build/obj/plic.o: ../hal/cpu/irq.h ../hal/cpu/cpu.h
../../../build/obj/plic.o: ../hal/arch/chardev.h ../kernel/kdev.h
../../../build/obj/plic.o: ../kernel/kmemory.h ../kernel/kthread.h
../../../build/obj/plic.o: ../kernel/ksynchro.h ../hal/arch/icu.h
../../../build/obj/soclib-timer.o: ../drivers/timer/soclib-timer.h
../../../build/obj/soclib-timer.o: ../hal/arch/timer.h
../../../build/obj/soclib-timer.o: ../kernel/klibc.h
../../../build/obj/soclib-timer.o: ../common/debug_off.h
../../../build/obj/soclib-timer.o: ../common/debug_on.h
../../../build/obj/soclib-timer.o: ../common/cstd.h ../common/list.h
../../../build/obj/soclib-timer.o: ../common/esc_code.h
../../../build/obj/soclib-timer.o: ../common/errno.h
../../../build/obj/soclib-timer.o: ../common/syscalls.h
../../../build/obj/soclib-timer.o: ../common/usermem.h
../../../build/obj/soclib-timer.o: ../hal/cpu/atomic.h
../../../build/obj/soclib-timer.o: ../hal/cpu/cache.h
../../../build/obj/soclib-timer.o: ../hal/arch/platform.h
../../../build/obj/soclib-timer.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/soclib-timer.o: ../hal/cpu/cpu.h
../../../build/obj/soclib-timer.o: ../hal/arch/chardev.h
../../../build/obj/soclib-timer.o: ../kernel/kdev.h ../kernel/kmemory.h
../../../build/obj/soclib-timer.o: ../kernel/kthread.h
../../../build/obj/soclib-timer.o: ../kernel/ksynchro.h
../../../build/obj/clint-timer.o: ../drivers/timer/clint-timer.h
../../../build/obj/clint-timer.o: ../hal/arch/timer.h
../../../build/obj/soclib-tty.o: ../drivers/chardev/soclib-tty.h
../../../build/obj/soclib-tty.o: ../hal/arch/chardev.h
../../../build/obj/soclib-tty.o: ../common/errno.h ../common/list.h
../../../build/obj/soclib-tty.o: ../kernel/kdev.h ../kernel/klibc.h
../../../build/obj/soclib-tty.o: ../common/debug_off.h
../../../build/obj/soclib-tty.o: ../common/debug_on.h
../../../build/obj/soclib-tty.o: ../common/cstd.h
../../../build/obj/soclib-tty.o: ../common/esc_code.h
../../../build/obj/soclib-tty.o: ../common/syscalls.h
../../../build/obj/soclib-tty.o: ../common/usermem.h
../../../build/obj/soclib-tty.o: ../hal/cpu/atomic.h
../../../build/obj/soclib-tty.o: ../hal/cpu/cache.h
../../../build/obj/soclib-tty.o: ../hal/arch/platform.h
../../../build/obj/soclib-tty.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/soclib-tty.o: ../hal/cpu/cpu.h ../kernel/kmemory.h
../../../build/obj/soclib-tty.o: ../kernel/kthread.h
../../../build/obj/soclib-tty.o: ../kernel/ksynchro.h
../../../build/obj/ns16550.o: ../drivers/chardev/ns16550.h
../../../build/obj/ns16550.o: ../hal/arch/chardev.h ../common/errno.h
../../../build/obj/ns16550.o: ../common/list.h ../kernel/kdev.h
../../../build/obj/ns16550.o: ../kernel/klibc.h ../common/debug_off.h
../../../build/obj/ns16550.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/ns16550.o: ../common/esc_code.h
../../../build/obj/ns16550.o: ../common/syscalls.h
../../../build/obj/ns16550.o: ../common/usermem.h ../hal/cpu/atomic.h
../../../build/obj/ns16550.o: ../hal/cpu/cache.h
../../../build/obj/ns16550.o: ../hal/arch/platform.h
../../../build/obj/ns16550.o: ../hal/cpu/thread.h ../hal/cpu/irq.h
../../../build/obj/ns16550.o: ../hal/cpu/cpu.h ../kernel/kmemory.h
../../../build/obj/ns16550.o: ../kernel/kthread.h
../../../build/obj/ns16550.o: ../kernel/ksynchro.h
