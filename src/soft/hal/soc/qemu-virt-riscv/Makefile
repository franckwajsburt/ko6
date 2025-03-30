#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2023-07-11
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     hal/soc/qemu-virt-riscv/Makefile
# \author   Nolan Bled
# \brief    Makefile to produce the kernel executable: kernel.x
#
#--------------------------------------------------------------------------------------------------

# General settings, directories and tools specific to the SOC 
# --------------------------------------------------------------------------------------------------

VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()

SOC    ?= qemu-virt-riscv#			defaut SOC name

SRCDIR  = $(ko6)/src/soft#			root source dir, allow us to make include via absolute path
BLDDIR	= $(ko6)/build#				build directory
LIBDIR	= .#						library directory (where is ldscript)
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
SOCDIR	= $(CURDIR)#				current soc hardware abstraction layer (hal) files directory
OBJDIR	= $(BLDDIR)/kobj#			kernel object files directory
PDFDIR	= $(BLDDIR)/kpdf#			kernel pdf files directory

include $(SRCDIR)/hal/soc/$(SOC)/$(SOC).mk

# Sources files
# --------------------------------------------------------------------------------------------------

SRC = soc.c
LDSCRIPT= kernel.ld#				ldscript is hal dependent

# Targets files defined from the source files
# --------------------------------------------------------------------------------------------------

SRCS    = $(filter %.S,$(SRC))
SRCC    = $(filter %.c,$(SRC))
OBJS    = $(subst .S,.o,$(foreach src,$(SRCS),$(notdir $(src))))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJS) $(OBJC))
OBJ 	+= $(OBJDIR)/cstd.o $(OBJDIR)/errno.o
OBJ 	+= $(OBJDIR)/plic.o
OBJ 	+= $(OBJDIR)/clint-timer.o $(OBJDIR)/ns16550.o
OBJ 	+= $(OBJDIR)/atomic.o $(OBJDIR)/cache.o $(OBJDIR)/cpuregs.o
OBJ 	+= $(OBJDIR)/entry.o $(OBJDIR)/irq.o $(OBJDIR)/kpanica.o
OBJ 	+= $(OBJDIR)/kpanica.o $(OBJDIR)/kpanicc.o $(OBJDIR)/threada.o
OBJ 	+= $(OBJDIR)/threadc.o
OBJ 	+= $(OBJDIR)/kinit.o $(OBJDIR)/klibc.o $(OBJDIR)/kmemory.o
OBJ 	+= $(OBJDIR)/ksynchro.o $(OBJDIR)/ksyscalls.o $(OBJDIR)/kthread.o
OBJ 	+= $(OBJDIR)/kdev.o $(OBJDIR)/kirq.o
OBJ 	+= $(OBJDIR)/libfdt.a
OBJDS   = $(addsuffix .s,$(OBJ))
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  the pdf file has the name of directory
BIN     = $(BLDDIR)/kernel-$(SOC).x

# Tools parameters
# --------------------------------------------------------------------------------------------------

CFLAGS += -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I. -I$(SRCDIR)#			directories where include files like <file.h> are located
CFLAGS += -DVERBOSE=$(VERBOSE)#		verbose if 1, can be toggled with #include <debug_{on,off}.h>

# Simple shell scripts
# --------------------------------------------------------------------------------------------------

.PHONY: help compil exec debug pdf clean _mkdir _depend

help:
	@echo ""
	@echo "Usage : make <compil|clean|pdf>"
	@echo ""
	@echo "        compil  : compile all sources"
	@echo "        clean   : clean all compiled files"
	@echo "        pdf     : generate $(PDF) with all source files"
	@echo ""

compil: _depend _mkdir $(BIN)

exec: compil
	xterm -bg black -fg green -n ko6 -geometry 80x50 -l -lf xterm.log \
		  -e qemu-system-riscv32 -M virt \
						-nographic \
						-bios none \
						-kernel $(BLDDIR)/kernel-$(SOC).x \
						-device loader,file=$(BLDDIR)/$(APP).x \
						-d guest_errors
debug: compil
	xterm -e qemu-system-riscv32 -M virt \
						-nographic \
						-bios none \
						-kernel $(BLDDIR)/kernel-$(SOC).x \
						-device loader,file=$(BLDDIR)/$(APP).x \
						-s \
						-S

_mkdir:
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
	@echo "- clean $(notdir $(BIN)) and related build files"
	-rm *~ $(OBJ) $(OBJDS) $(BIN) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Building rules
# --------------------------------------------------------------------------------------------------

$(BIN) : $(LDSCRIPT) $(OBJ)
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
_depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCC) $(SRCS)
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/obj/cpu/:/obj/:' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# --------------------------------------------------------------------------------------------------
