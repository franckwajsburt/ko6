#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date      2023-07-11
# | / /(     )/ _ \   	\copyright 2021-2022 Sorbonne University
# |_\_\ x___x \___/                https://opensource.org/licenses/MIT
#
# \file     hal/soc/almo1-mips/Makefile
# \author   Franck Wajsburt, Nolan Bled
# \brief    Makefile to build soc-specific files, and link every files in 
#			the kernel
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

SOC     = almo1-mips# 				soc name
VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()
SRCDIR  = ../../..#					root source dir, allow us to make include via absolute path
BLDDIR	= ../../../../../build#		build directory
BINDIR 	= ../../../../../bin#		tools directory
LIBDIR	= .#						library directory (where is ldscript)
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
SOCDIR	= $(CURDIR)#				current soc hardware abstraction layer (hal) files directory
OBJDIR	= $(BLDDIR)/obj#			object files directory
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory
BIN 	= $(BLDDIR)/kernel-$(SOC).x
SX 		= $(SOC).x

# Sources
# --------------------------------------------------------------------------------------------------

SRC = soc.c dtb.S
LDSCRIPT = kernel.ld

# Targets
# --------------------------------------------------------------------------------------------------

SRCS    = $(filter %.S,$(SRC))
SRCC    = $(filter %.c,$(SRC))
OBJS    = $(subst .S,.o,$(foreach src,$(SRCS),$(notdir $(src))))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJS) $(OBJC))
OBJ 	+= $(OBJDIR)/cstd.o $(OBJDIR)/errno.o
OBJ 	+= $(OBJDIR)/soclib-icu.o $(OBJDIR)/soclib-dma.o
OBJ 	+= $(OBJDIR)/soclib-timer.o $(OBJDIR)/soclib-tty.o
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

# Tools
# --------------------------------------------------------------------------------------------------

include $(SRCDIR)/hal/soc/almo1-mips/almo1-mips.mk

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

.PHONY: help compil exec debug mkdir pdf clean depend

help:
	@echo ""
	@echo "Usage : make <compil|clean|pdf>"
	@echo ""
	@echo "        compil  : compile all sources"
	@echo "        clean   : clean all compiled files"
	@echo "        pdf     : generate $(PDF) with all source files"
	@echo ""

compil: depend mkdir mkdtb $(OBJ) $(BIN) 

mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(OBJDIR) || (echo "- mkdir   $(OBJDIR)" ; mkdir $(OBJDIR))

mkdtb:
	dtc -I dts -O dtb -o almo1.dtb almo1.dts

exec: compil
	$(BINDIR)/$(SX) -KERNEL $(BLDDIR)/kernel-$(SOC).x -APP $(BLDDIR)/$(APP).x -NTTYS $(NTTYS) -NCPUS $(NCPUS)

debug: compil
	$(BINDIR)/$(SX) -KERNEL $(BLDDIR)/kernel-$(SOC).x -APP $(BLDDIR)/$(APP).x -NTTYS $(NTTYS) -NCPUS $(NCPUS)\
          -DEBUG $(FROM) -NCYCLES $(LAST) > $(DLOG)
	tracelog $(SWDIR)/tags $(BLDDIR)/*.x.s $(DLOG)

pdf:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(PDFDIR) || (echo "- mkdir   $(PDFDIR)" ; mkdir $(PDFDIR))
	a2ps -T4 -2 -M A4 -A fill -o - -l100 Makefile $(SRC) 2> $(PDF).log |\
	ps2pdf -sPAPERSIZE=a4 - $(PDF);\
	echo "- create $(PDF)";\
	sed 's/^/  - /;s/\].*/\]/' $(PDF).log

clean:
	@echo "- clean $(SOC) and related build files"
	-rm *~ *.dtb $(BIN) $(OBJ) $(OBJDS) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Rules with automatic variables to produce the executable
# --------------------------------------------------------------------------------------------------

$(BIN) : $(LDSCRIPT) $(OBJ)
	@echo "- link    --> "$(notdir $@)
	$(LD) $(LDFLAGS) -o $@ -T $^
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
	rm Makefile.bak

# --------------------------------------------------------------------------------------------------
