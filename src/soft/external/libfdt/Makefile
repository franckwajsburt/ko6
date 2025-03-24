#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date    	2025-02-23
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     external/libfdt/Makefile
# \author   Nolan Bled
# \brief    Makefile to generate the libfdt static library
#
#--------------------------------------------------------------------------------------------------

#--------------------------------------------------------------------------------------------------
# SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
# Makefile.libfdt
#
# This is not a complete Makefile of itself.  Instead, it is designed to
# be easily embeddable into other systems of Makefiles.
#
LIBFDT_soname 	= libfdt.$(SHAREDLIB_EXT).1
LIBFDT_INCLUDES = fdt.h libfdt.h libfdt_env.h
LIBFDT_VERSION 	= version.lds
LIBFDT_SRCS 	= fdt.c fdt_ro.c fdt_wip.c fdt_sw.c fdt_rw.c fdt_strerror.c fdt_empty_tree.c \
			  	  fdt_addresses.c fdt_overlay.c fdt_check.c
LIBFDT_OBJS 	= $(LIBFDT_SRCS:%.c=%.o)
LIBFDT_LIB 		= libfdt-$(DTC_VERSION).$(SHAREDLIB_EXT)
#--------------------------------------------------------------------------------------------------

SOC    ?= almo1-mips
SRCDIR 	= $(ko6)/src/soft#			directory of all sources
BLDDIR	= $(ko6)/build#				build directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
OBJDIR	= $(BLDDIR)/kobj#			kernel object files directory
PDFDIR	= $(BLDDIR)/kpdf#			kernel pdf files directory
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  the pdf file has the name of directory

# Objects from Makefile.libfdt
LIBFDT_OBJS:=$(addprefix $(OBJDIR)/,$(LIBFDT_OBJS))

include $(SRCDIR)/hal/soc/$(SOC)/$(SOC).mk

CFLAGS += -c#							stop after compilation, then produce .o
CFLAGS += -Wall -Werror#				gives almost all C warnings and considers them to be errors
CFLAGS += -std=c99#						define of syntax version of C
CFLAGS += -fno-common#					do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#					do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#			only use of stack pointer ($29)
CFLAGS += -O3#							full optimisation mode of compiler
CFLAGS += -I.
CFLAGS += -I$(SRCDIR)

all: compil

compil: depend mkdir $(OBJDIR)/libfdt.a

mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(OBJDIR) || (echo "- mkdir   $(OBJDIR)" ; mkdir $(OBJDIR))

$(OBJDIR)/libfdt.a: $(LIBFDT_OBJS)
	@echo "- archive --> "$(notdir $@)
	ar -rc $@ $(LIBFDT_OBJS)
	$(RL) $@

$(OBJDIR)/%.o: %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<

pdf:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(PDFDIR) || (echo "- mkdir   $(PDFDIR)" ; mkdir $(PDFDIR))
	a2ps -T4 -2 -M A4 -A fill -o - -l100 Makefile $(LIBFDT_SRCS) 2> $(PDF).log |\
	ps2pdf -sPAPERSIZE=a4 - $(PDF);\
	echo "- create $(PDF)";\
	sed 's/^/  - /;s/\].*/\]/' $(PDF).log

clean:
	@echo "- clean $(CURDIR) and related build files"
	-rm *~ *.dtb $(LIBFDT_OBJS) $(OBJDIR)/libfdt.a $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(LIBFDT_SRCS)
	rm Makefile.bak

# ------------------------------------------------------------------------------------------------
