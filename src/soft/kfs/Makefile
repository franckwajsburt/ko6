#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2023-08-02
# | / /(     )/ _ \   	\copyright  2021-2023 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     kfs/Makefile
# \author   Franck Wajsburt
# \brief    Makefile to produce kfs objects files and ktools application
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

SOC    ?= almo1-mips#				defaut SOC name
VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()
BLDDIR	= ../../../build#			build directory
BINDIR	= ../../../bin#				build directory
SRCDIR 	= ..#						directory of all sources
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
OBJDIR	= $(BLDDIR)/obj#			object files directory
KFSTDIR = $(BLDDIR)/ktools#			object files directory for ktools
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory
KFST	= $(BINDIR)/ktools#			ktools application

# Sources
# --------------------------------------------------------------------------------------------------

SRC     = kfs.c kfs.h
SRCKFST = $(SRC) ktools.c

# Targets
# --------------------------------------------------------------------------------------------------

SRCC    = $(filter %.c %.S,$(SRC))
SRCKFSTC= $(filter %.c %.S,$(SRCKFST))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJKFSTC= $(subst .c,.o,$(foreach src,$(SRCKFSTC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJC))
OBJKFST = $(addprefix $(KFSTDIR)/,$(OBJKFSTC))
OBJDS   = $(addsuffix .s,$(OBJ))
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  the pdf file has the name of directory

# Tools
# --------------------------------------------------------------------------------------------------

include $(SRCDIR)/hal/soc/$(SOC)/$(SOC).mk

CFLAGS += -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I.#						directories where include files like <file.h> are located
CFLAGS += -DVERBOSE=$(VERBOSE)#		verbose if 1, can be toggled with #include <debug_{on,off}.h>

HOSTCC  	= gcc#									native compiler of host computer
HOSTCFLAGS	= -c -Wall -I. -std=c99 -O3 -g -DHOST#	compiler flag for native compiler

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
	@echo "OBJ= "$(OBJ)
	@echo "KFST= "$(KFST)
	@echo "OBJKFST= "$(OBJKFST)

compil: depend mkdir $(OBJ) $(KFST)

mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(OBJDIR) || (echo "- mkdir   $(OBJDIR)" ; mkdir $(OBJDIR))
	test -d $(KFSTDIR)|| (echo "- mkdir   $(KFSTDIR)" ; mkdir $(KFSTDIR))

pdf:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(PDFDIR) || (echo "- mkdir   $(PDFDIR)" ; mkdir $(PDFDIR))
	a2ps -T4 -2 -M A4 -A fill -o - -l100 Makefile $(SRC) 2> $(PDF).log |\
	ps2pdf -sPAPERSIZE=a4 - $(PDFDIR)/$(PDF);\
	echo "- create $(PDF)";\
	sed 's/^/  - /;s/\].*/\]/' $(PDF).log

clean:
	@echo "- clean $(CURDIR) and related build files"
	-rm *~ $(OBJ) $(OBJDS) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Rules with automatic variables
# --------------------------------------------------------------------------------------------------

$(OBJDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(KFST) : $(OBJKFST)
	@echo "- compil  --> "$(notdir $@)
	$(HOSTCC) -o $@ $^

$(KFSTDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(HOSTCC) -o $@ $(HOSTCFLAGS) $<

# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCKFST)
	rm Makefile.bak

# --------------------------------------------------------------------------------------------------
