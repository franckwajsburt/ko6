#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date      2023-07-11
# | / /(     )/ _ \   	\copyright 2021-2022 Sorbonne University
# |_\_\ x___x \___/                https://opensource.org/licenses/MIT
#
# \file     platforms/almo1/Makefile
# \author   Franck Wajsburt, Nolan Bled
# \brief    Makefile to produce the kernel executable: kernel.x
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

NTTYS  ?= 1#						default number of ttys
NCPUS  ?= 1#						default number of CPUS
VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()
SRCDIR  = ../..#					root source dir, allow us to make include via absolute path
BLDDIR	= ../../../../build#		build directory
LIBDIR	= .#						library directory (where is ldscript)
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
SOCDIR	= $(CURDIR)#					current soc hardware abstraction layer (hal) files directory
OBJDIR	= $(BLDDIR)/obj#			object files directory
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory

# Sources
# --------------------------------------------------------------------------------------------------

SRC = cpuc.c cpu/atomic.S cpu/irq.S cpu/thread.S cpu/cpu.S cpu/cache.S
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
OBJ 	+= $(OBJDIR)/chardev.o $(OBJDIR)/irq.o $(OBJDIR)/dev.o
OBJ 	+= $(OBJDIR)/libfdt.a
OBJDS   = $(addsuffix .s,$(OBJ))
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  the pdf file has the name of directory
BIN     = $(BLDDIR)/$(CURDIR).x

# Tools
# --------------------------------------------------------------------------------------------------

include $(SRCDIR)/platforms/virt/Makefile.virt

CFLAGS += -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I. -I$(SRCDIR)#			directories where include files like <file.h> are located
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
	@echo $(OBJ)
	@echo "- link    --> "$(notdir $@)
	$(LD) $(LDFLAGS) -o $@ -T $^
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : cpu/%.S
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCC) $(SRCS)
# DEPENDENCIES

../../../../build/obj/cpuc.o: ../../platforms/virt/cpu.h ../../kernel/klibc.h
../../../../build/obj/cpuc.o: ../../common/debug_off.h
../../../../build/obj/cpuc.o: ../../common/debug_on.h ../../common/cstd.h
../../../../build/obj/cpuc.o: ../../common/list.h ../../common/esc_code.h
../../../../build/obj/cpuc.o: ../../common/errno.h ../../common/syscalls.h
../../../../build/obj/cpuc.o: ../../common/usermem.h ../../hal/cpu/atomic.h
../../../../build/obj/cpuc.o: ../../hal/cpu/cache.h ../../hal/platform.h
../../../../build/obj/cpuc.o: ../../hal/cpu/irq.h ../../hal/cpu/cpu.h
../../../../build/obj/cpuc.o: ../../hal/chardev.h ../../hal/dev.h
../../../../build/obj/cpuc.o: ../../kernel/kmemory.h ../../kernel/kthread.h
../../../../build/obj/cpuc.o: ../../kernel/ksynchro.h
../../../../build/obj/cpu/thread.o: ../../platforms/virt/cpu.h
../../../../build/obj/cpu/cpu.o: ../../platforms/virt/cpu.h
