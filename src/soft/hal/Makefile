#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2025-04-20
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/     \licence    https://opensource.org/licenses/MIT
#
# \file     hal/Makefile
# \author   Nolan Bled, Franck Wajsburt
# \brief    Makefile to compile the SoC specific code for devices & CPU : hal
#
#--------------------------------------------------------------------------------------------------

# General settings, directories and tools specific to the SOC
# --------------------------------------------------------------------------------------------------

VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()

SOC    ?= almo1-mips#				defaut SOC name
CPU	   ?= mips#						defaut CPU

SRCDIR 	= $(ko6)/src/soft#			directory of all sources
HALDIR  = $(SRCDIR)/hal#			HAL directory (current)
COMDIR	= $(SRCDIR)/common#			user & kernel common files directory
BLDDIR	= $(ko6)/build#				build directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
OBJDIR	= $(BLDDIR)/kobj#			kernel object files directory
PDFDIR	= $(BLDDIR)/kpdf#			kernel pdf files directory

include $(SRCDIR)/hal/soc/$(SOC)/$(SOC).mk

# Sources Files
# --------------------------------------------------------------------------------------------------

SRC     = $(wildcard devices/blockdev/*.c) \
		  $(wildcard devices/chardev/*.c) \
		  $(wildcard devices/dma/*.c) \
		  $(wildcard devices/tty/*.c) \
		  $(wildcard devices/icu/*.c) \
		  $(wildcard devices/timer/*.c) \
		  $(wildcard cpu/$(CPU)/*.S) \
		  $(wildcard cpu/$(CPU)/*.c) \

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

include $(SRCDIR)/hal/soc/$(SOC)/$(SOC).mk

CFLAGS += -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I..#						directories where include files like <file.h> are located
CFLAGS += -DVERBOSE=$(VERBOSE)#		verbose if 1, can be toggled with #include <debug_{on,off}.h>
CFLAGS += -DKO6VER="\"$(KO6VER)\""# last commit

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
	make -C soc/$(SOC) compil

_mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(OBJDIR) || (echo "- mkdir   $(OBJDIR)" ; mkdir $(OBJDIR))

pdf:
	make -C soc/$(SOC) pdf
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(PDFDIR) || (echo "- mkdir   $(PDFDIR)" ; mkdir $(PDFDIR))
	a2ps -T4 -2 -M A4 -A fill -o - -l100 Makefile $(SRC) 2> $(PDF).log |\
	ps2pdf -sPAPERSIZE=a4 - $(PDF);\
	echo "- create $(PDF)";\
	sed 's/^/  - /;s/\].*/\]/' $(PDF).log

clean:
	make -C soc/$(SOC) clean
	@echo "- clean $(CURDIR) and related build files"
	-rm *~ *.dtb $(OBJ) $(OBJDS) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Building rules
# --------------------------------------------------------------------------------------------------

$(OBJDIR)/%.o : $(HALDIR)/devices/blockdev/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(HALDIR)/devices/chardev/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(HALDIR)/devices/dma/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(HALDIR)/devices/icu/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(HALDIR)/devices/timer/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(HALDIR)/cpu/$(CPU)/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)%.o : $(HALDIR)/cpu/$(CPU)/%.S
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(HALDIR)/soc/$(SOC)/%.c
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
