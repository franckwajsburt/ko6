#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2025-02-26
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     ulib/Makefile
# \author   Franck Wajsburt
# \brief    Makefile to produce user library object file
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

CPU	   ?= mips
SOC    ?= almo1-mips#				defaut SOC name
VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()
BLDDIR	= $(ko6)/build#				build directory
SRCDIR 	= $(ko6)/src/soft#			directory of all sources
COMDIR	= $(SRCDIR)/common#			user & kernel common files directory
HALDIR	= $(SRCDIR)/hal#			hardware abstraction layer (hal) files directory
SOCDIR	= $(HALDIR)/soc#			current soc hardware abstraction layer (hal) files directory
CPUDIR	= $(HALDIR)/cpu#			current cpu hardware abstraction layer (hal) files directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
OBJDIR	= $(BLDDIR)/uobj#			user object files directory
PDFDIR	= $(BLDDIR)/updf#			user pdf files directory

# Sources
# --------------------------------------------------------------------------------------------------

SRC     = $(COMDIR)/syscalls.h $(COMDIR)/list.h $(COMDIR)/usermem.h
SRC    += $(COMDIR)/debug_on.h $(COMDIR)/debug_off.h
SRC    += $(COMDIR)/errno.h $(COMDIR)/errno.c
SRC    += $(COMDIR)/cstd.h $(COMDIR)/cstd.c
SRC    += $(COMDIR)/ctype.h $(COMDIR)/ctype.c
SRC    += $(COMDIR)/ht_prob.h $(COMDIR)/ht_prob.c
SRC    += $(CPUDIR)/$(CPU)/cpu_user.S
SRC    += crt0.c libc.h libc.c pthread.h pthread.c memory.h memory.c

# Targets
# --------------------------------------------------------------------------------------------------

SRCC    = $(filter %.c %.S,$(SRC))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJC   := $(subst .S,.o,$(foreach src,$(OBJC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJC))
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
CFLAGS += -I. -I.. -I$(COMDIR)#		directories where include files like <file.h> are located
CFLAGS += -I$(CPUDIR) -I$(SOCDIR)#	directory where hal include files are located
CFLAGS += -DVERBOSE=$(VERBOSE)#		verbose if 1, can be toggled with #include <debug_{on,off}.h>

# Rules (here they are used such as simple shell scripts)
# --------------------------------------------------------------------------------------------------

.PHONY: help compil tags pdf clean depend

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

# Rules with automatic variables
# --------------------------------------------------------------------------------------------------

$(OBJDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(COMDIR)/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o: $(CPUDIR)/$(CPU)/cpu_user.S
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRC)
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/../common/:/:' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# --------------------------------------------------------------------------------------------------
