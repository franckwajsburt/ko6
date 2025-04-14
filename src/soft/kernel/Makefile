#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2025-04-05
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/     \licence    https://opensource.org/licenses/MIT
#
# \file     kernel/Makefile
# \author   Franck Wajsburt
# \brief    Makefile to compile the kernel object files (not the executable)
#
#--------------------------------------------------------------------------------------------------

# General settings, directories and tools specific to the SOC  
# --------------------------------------------------------------------------------------------------

VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()

SOC    ?= almo1-mips#				defaut SOC name

BLDDIR	= $(ko6)/build#i		 	build directory
SRCDIR	= $(ko6)/src/soft#			source directories
COMDIR	= $(SRCDIR)/common#			user & kernel common files directory
HALDIR	= $(SRCDIR)/hal#			hardware abstraction layer (hal) files directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
OBJDIR	= $(BLDDIR)/kobj#			kernel object files directory
PDFDIR	= $(BLDDIR)/kpdf#			kernel pdf files directory
XLIBDIR = $(ko6)/lib#				external libraries directory

include $(SRCDIR)/hal/soc/$(SOC)/$(SOC).mk

# Sources files
# --------------------------------------------------------------------------------------------------

SRC     = $(COMDIR)/debug_on.h $(COMDIR)/debug_off.h
SRC    += $(COMDIR)/syscalls.h ksyscalls.c
SRC    += $(COMDIR)/usermem.h $(COMDIR)/list.h
SRC    += $(COMDIR)/errno.h $(COMDIR)/errno.c $(COMDIR)/esc_code.h
SRC    += $(COMDIR)/cstd.c $(COMDIR)/cstd.h
SRC    += $(COMDIR)/ctype.c $(COMDIR)/ctype.h
SRC    += $(COMDIR)/htopen.c $(COMDIR)/htopen.h
SRC    += kmemkernel.c kmemkernel.h
SRC    += kmemuser.c kmemuser.h
SRC    += klibc.c klibc.h
SRC    += kthread.c kthread.h
SRC    += kinit.c kdev.c kirq.c
SRC    += ksynchro.c ksynchro.h
SRC    += kshell.c kshell.h

# Targets files defined from the source files
# --------------------------------------------------------------------------------------------------

SRCS    = $(filter %.S,$(SRC))
SRCC    = $(filter %.c,$(SRC))
OBJS    = $(subst .S,.o,$(foreach src,$(SRCS),$(notdir $(src))))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJS) $(OBJC))
OBJ 	+= $(OBJDIR)/libfdt.a

OBJDS   = $(addsuffix .s,$(OBJ))
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  the pdf file has the name of directory

# Tools parameters
# --------------------------------------------------------------------------------------------------

CFLAGS += -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I.#						directories where include files like <file.h> are located
CFLAGS += -I..#						directories where include files like <file.h> are located
CFLAGS += -I$(COMDIR)#				directories where include files like <file.h> are located
CFLAGS += -I$(XLIBDIR)/libfdt#		include external libraries (specifically libfdt.h)
CFLAGS += -DVERBOSE=$(VERBOSE)#		verbose if 1, can be toggled with #include <debug_{on,off}.h>
CFLAGS += -D_KERNEL_#				to tell gcc we compile for ko6

# Simple shell scripts
# --------------------------------------------------------------------------------------------------

.PHONY: help compil pdf clean _mkdir _depend

help:
	@echo ""
	@echo "Usage : make <compil|clean|pdf>"
	@echo ""
	@echo "        compil  : compile all sources"
	@echo "        clean   : clean all compiled files"
	@echo "        pdf     : generate $(PDF) with all source files"
	@echo ""

compil: _depend _mkdir $(OBJ)

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
	@echo "- clean $(CURDIR) and related build files"
	-rm *~ *.dtb $(OBJ) $(OBJDS) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Building rules
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
_depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCC) $(SRCS)
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/../common/:/:' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# --------------------------------------------------------------------------------------------------
