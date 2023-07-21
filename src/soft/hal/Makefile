#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date      2022-07-03
# | / /(     )/ _ \   	\copyright 2021-2022 Sorbonne University
# |_\_\ x___x \___/                https://opensource.org/licenses/MIT
#
# \file     drivers/Makefile
# \author   Nolan Bled
# \brief    Makefile to produce the HAL helper functions (TTYs and IRQs)
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

SOC 	?= almo1
VERBOSE	?= 1

BLDDIR	= ../../../build#			build directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
SRCDIR	= ..#						root of the source files
OBJDIR	= $(BLDDIR)/obj#			object files directory
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory

# Sources
# --------------------------------------------------------------------------------------------------

SRC		= dev.c chardev.c cpu/irq.c

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

include $(SRCDIR)/platforms/$(SOC)/Makefile.$(SOC)

CFLAGS += -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I$(SRCDIR)#				directories where include files like <file.h> are located
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
	-rm *~ $(OBJ) $(OBJDS) $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Rules with automatic variables to produce the executable
# --------------------------------------------------------------------------------------------------

$(OBJDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : cpu/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

