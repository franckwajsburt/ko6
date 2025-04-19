#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2025-04-05
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/     \licence    https://opensource.org/licenses/MIT
#
# \file     hal/soc/almo1-mips/Makefile
# \author   Franck Wajsburt, Nolan Bled
# \brief    Makefile to compile SoC specific files, build the kernel and launch the SoC execution
#
#--------------------------------------------------------------------------------------------------

# Default SoC options
# --------------------------------------------------------------------------------------------------

NTTYS  ?= 2#						default number of ttys
NCPUS  ?= 1#						default number of CPUS
FROM   ?= 0#						first cycle to trace
LAST   ?= 1000000#					last cycle to execute

# General settings, directories and tools specific to the SOC  
# --------------------------------------------------------------------------------------------------

VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()

SOC     = almo1-mips# 				soc name

SRCDIR	= $(ko6)/src/soft#			root source dir, allow us to make include via absolute path
BLDDIR	= $(ko6)/build#				build directory
BINDIR 	= $(ko6)/bin#				tools directory
LIBDIR	= .#						library directory (where is ldscript)
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
SOCDIR	= $(CURDIR)#				current soc hardware abstraction layer (hal) files directory
OBJDIR	= $(BLDDIR)/kobj#			kernel object files directory
PDFDIR	= $(BLDDIR)/kpdf#			kernel pdf files directory
EXEC 	= $(BINDIR)/$(SOC).x#		prototype simulator
DLOG    = /tmp/ko6-debug.log#       debug file

include $(SRCDIR)/hal/soc/almo1-mips/almo1-mips.mk

# Sources files 
# --------------------------------------------------------------------------------------------------

SRC = soc.c dtb.S
LDSCRIPT = kernel.ld

# Targets files defined from the source files
# --------------------------------------------------------------------------------------------------

BIN 	= $(BLDDIR)/kernel-$(SOC).x#	kernel

SRCS    = $(filter %.S,$(SRC))
SRCC    = $(filter %.c,$(SRC))
OBJS    = $(subst .S,.o,$(foreach src,$(SRCS),$(notdir $(src))))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJS) $(OBJC))
OBJ 	+= $(OBJDIR)/cstd.o $(OBJDIR)/ctype.o $(OBJDIR)/htopen.o $(OBJDIR)/errno.o
OBJ 	+= $(OBJDIR)/soclib-icu.o $(OBJDIR)/soclib-dma.o
OBJ 	+= $(OBJDIR)/soclib-timer.o $(OBJDIR)/soclib-tty.o
OBJ 	+= $(OBJDIR)/soclib-bd.o
OBJ 	+= $(OBJDIR)/atomic.o $(OBJDIR)/cache.o $(OBJDIR)/cpuregs.o
OBJ 	+= $(OBJDIR)/entry.o $(OBJDIR)/irq.o $(OBJDIR)/kpanica.o
OBJ 	+= $(OBJDIR)/kpanica.o $(OBJDIR)/kpanicc.o $(OBJDIR)/threada.o
OBJ 	+= $(OBJDIR)/threadc.o
OBJ 	+= $(OBJDIR)/kinit.o $(OBJDIR)/klibc.o 
OBJ		+= $(OBJDIR)/kmemkernel.o $(OBJDIR)/kmemuser.o
OBJ 	+= $(OBJDIR)/ksynchro.o $(OBJDIR)/ksyscalls.o $(OBJDIR)/kthread.o
OBJ 	+= $(OBJDIR)/kdev.o $(OBJDIR)/kirq.o
OBJ 	+= $(OBJDIR)/pvfs.o $(OBJDIR)/pvfs.o
OBJ		+= $(OBJDIR)/kshell.o
OBJ		+= $(OBJDIR)/kblockio.o
OBJ 	+= $(OBJDIR)/libfdt.a
OBJDS   = $(addsuffix .s,$(OBJ))
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  	the pdf file has the name of directory

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

compil: _mkdir _depend mkdtb $(OBJ) $(BIN) 

_mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(OBJDIR) || (echo "- mkdir   $(OBJDIR)" ; mkdir $(OBJDIR))

mkdtb:
	dtc -I dts -O dtb -o almo1.dtb almo1.dts

exec: compil
	cd $(BLDDIR);\
	$(EXEC) -KERNEL $(BLDDIR)/kernel-$(SOC).x -APP $(BLDDIR)/$(APP)/$(APP).x\
					-NTTYS $(NTTYS) -NCPUS $(NCPUS) $(DISK) $(FBF)

debug: compil
	cd $(BLDDIR);\
	$(EXEC) -KERNEL $(BLDDIR)/kernel-$(SOC).x -APP $(BLDDIR)/$(APP)/$(APP).x\
			-NTTYS $(NTTYS) -NCPUS $(NCPUS) $(DISK) $(FBF)\
			-DEBUG $(FROM) -NCYCLES $(LAST) > $(DLOG)
	cd $(BLDDIR);\
	tracelog $(SRCDIR)/tags $(BLDDIR)/$(APP)/*.x.s $(BLDDIR)/*.x.s $(DLOG)

pdf:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(PDFDIR) || (echo "- mkdir   $(PDFDIR)" ; mkdir $(PDFDIR))
	a2ps -T4 -2 -M A4 -A fill -o - -l100 Makefile $(SRC) 2> $(PDF).log |\
	ps2pdf -sPAPERSIZE=a4 - $(PDF);\
	echo "- create $(PDF)";\
	sed 's/^/  - /;s/\].*/\]/' $(PDF).log

clean:
	@echo "- clean $(SOC) and related build files"
	-rm *~ 	segment.ld *.dtb $(BIN) $(OBJ) $(OBJDS) $(PDF) $(PDF).log \
			$(BLDDIR)/xterm* $(BLDDIR)/trace* $(BLDDIR)/label*  2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Building rules
# --------------------------------------------------------------------------------------------------

$(BIN) : $(LDSCRIPT) $(OBJ)
	@echo "- link    --> "$(notdir $@)
	sed "s/(APPNB)/0/" segs.ld > segment.ld
	$(LD) $(LDFLAGS) -o $@ -T $^
	truncate -s %4096 $(BIN)
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
_depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCC) $(SRCS)
	rm Makefile.bak

# --------------------------------------------------------------------------------------------------
