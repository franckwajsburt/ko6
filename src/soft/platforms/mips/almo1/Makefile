#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date      2023-07-11
# | / /(     )/ _ \   	\copyright 2021-2022 Sorbonne University
# |_\_\ x___x \___/                https://opensource.org/licenses/MIT
#
# \file     platforms/mips/almo1/Makefile
# \author   Franck Wajsburt, Nolan Bled
# \brief    Makefile to produce the kernel executable: kernel.x
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

NTTYS  ?= 1#						default number of ttys
NCPUS  ?= 1#						default number of CPUS
VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()
SRCDIR  = ../../..#					root source dir, allow us to make include via absolute path
BLDDIR	= ../../../../../build#		build directory
LIBDIR	= .#						library directory (where is ldscript)
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
SOCDIR	= $(CURDIR)#					current soc hardware abstraction layer (hal) files directory
OBJDIR	= $(BLDDIR)/obj#			object files directory
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory

# Sources
# --------------------------------------------------------------------------------------------------

SRC = cpua.S cpuc.c platform.c
LDSCRIPT= kernel.ld#				ldscript is hal dependent

# Targets
# --------------------------------------------------------------------------------------------------

SRCS    = $(filter %.S,$(SRC))
SRCC    = $(filter %.c,$(SRC))
OBJS    = $(subst .S,.o,$(foreach src,$(SRCS),$(notdir $(src))))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJS) $(OBJC))
OBJ 	+= $(OBJDIR)/cstd.o $(OBJDIR)/errno.o
OBJ 	+= $(OBJDIR)/soclib-dma.o $(OBJDIR)/soclib-icu.o
OBJ 	+= $(OBJDIR)/soclib-timer.o $(OBJDIR)/soclib-tty.o
OBJ 	+= $(OBJDIR)/kinit.o $(OBJDIR)/klibc.o $(OBJDIR)/kmemory.o
OBJ 	+= $(OBJDIR)/ksynchro.o $(OBJDIR)/ksyscalls.o $(OBJDIR)/kthread.o
OBJ 	+= $(OBJDIR)/tty.o $(OBJDIR)/irq.o $(OBJDIR)/dev.o
OBJDS   = $(addsuffix .s,$(OBJ))
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  the pdf file has the name of directory
BIN     = $(BLDDIR)/$(CURDIR).x

# Tools
# --------------------------------------------------------------------------------------------------

CC     = mipsel-unknown-elf-gcc#	compiler
LD     = mipsel-unknown-elf-ld#		linker
OD     = mipsel-unknown-elf-objdump#desassembler

CFLAGS  = -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -mips32r2#				define of MIPS version
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -G0#						do not use global data pointer ($28)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I. -I$(SRCDIR)#			directories where include files like <file.h> are located
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

compil: depend mkdir $(BIN)

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
	@echo "- clean up $(BIN) and related build files"
	-rm *~ $(OBJ) $(OBJDS) $(BIN) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Rules with automatic variables to produce the executable
# --------------------------------------------------------------------------------------------------

$(BIN) : $(LDSCRIPT) $(OBJ)
	@echo "- link    --> "$(notdir $@)
	$(LD) -o $@ -T $^
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : %.S
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCC) $(SRCS)
# DEPENDENCIES

../../../../../build/obj/cpuc.o: ../../../platforms/mips/almo1/cpu.h
../../../../../build/obj/cpuc.o: ../../../kernel/klibc.h
../../../../../build/obj/cpuc.o: ../../../common/debug_off.h
../../../../../build/obj/cpuc.o: ../../../common/debug_on.h
../../../../../build/obj/cpuc.o: ../../../common/cstd.h
../../../../../build/obj/cpuc.o: ../../../common/list.h
../../../../../build/obj/cpuc.o: ../../../common/esc_code.h
../../../../../build/obj/cpuc.o: ../../../common/errno.h
../../../../../build/obj/cpuc.o: ../../../common/syscalls.h
../../../../../build/obj/cpuc.o: ../../../common/usermem.h
../../../../../build/obj/cpuc.o: ../../../hal/cpu/atomic.h
../../../../../build/obj/cpuc.o: ../../../hal/cpu/cache.h
../../../../../build/obj/cpuc.o: ../../../hal/platform.h
../../../../../build/obj/cpuc.o: ../../../hal/cpu/irq.h
../../../../../build/obj/cpuc.o: ../../../hal/cpu/cpu.h ../../../hal/tty.h
../../../../../build/obj/cpuc.o: ../../../hal/dev.h ../../../kernel/kmemory.h
../../../../../build/obj/cpuc.o: ../../../kernel/kthread.h
../../../../../build/obj/cpuc.o: ../../../kernel/ksynchro.h
../../../../../build/obj/platform.o: ../../../drivers/timer/soclib-timer.h
../../../../../build/obj/platform.o: ../../../hal/timer.h
../../../../../build/obj/platform.o: ../../../drivers/icu/soclib-icu.h
../../../../../build/obj/platform.o: ../../../hal/icu.h
../../../../../build/obj/platform.o: ../../../drivers/tty/soclib-tty.h
../../../../../build/obj/platform.o: ../../../hal/tty.h
../../../../../build/obj/platform.o: ../../../common/errno.h
../../../../../build/obj/platform.o: ../../../common/list.h
../../../../../build/obj/platform.o: ../../../hal/dev.h
../../../../../build/obj/platform.o: ../../../kernel/klibc.h
../../../../../build/obj/platform.o: ../../../common/debug_off.h
../../../../../build/obj/platform.o: ../../../common/debug_on.h
../../../../../build/obj/platform.o: ../../../common/cstd.h
../../../../../build/obj/platform.o: ../../../common/esc_code.h
../../../../../build/obj/platform.o: ../../../common/syscalls.h
../../../../../build/obj/platform.o: ../../../common/usermem.h
../../../../../build/obj/platform.o: ../../../hal/cpu/atomic.h
../../../../../build/obj/platform.o: ../../../hal/cpu/cache.h
../../../../../build/obj/platform.o: ../../../hal/platform.h
../../../../../build/obj/platform.o: ../../../hal/cpu/irq.h
../../../../../build/obj/platform.o: ../../../hal/cpu/cpu.h
../../../../../build/obj/platform.o: ../../../kernel/kmemory.h
../../../../../build/obj/platform.o: ../../../kernel/kthread.h
../../../../../build/obj/platform.o: ../../../kernel/ksynchro.h
../../../../../build/obj/platform.o: ../../../drivers/dma/soclib-dma.h
../../../../../build/obj/platform.o: ../../../hal/dma.h
../../../../../build/obj/cpua.o: ../../../platforms/mips/almo1/cpu.h
../../../../../build/obj/cpua.o: ../../../common/syscalls.h
