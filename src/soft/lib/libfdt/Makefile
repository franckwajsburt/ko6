#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date      	2023-09-03
# | / /(     )/ _ \   	\copyright  2022-2023 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     lib/libfdt/Makefile
# \author   Nolan Bled
# \brief    Makefile to generate the libfdt static library
#
#--------------------------------------------------------------------------------------------------

include Makefile.libfdt

SOC		?= almo1-mips

SRCDIR	=	../..
BLDDIR	=	../../../../build
OBJDIR	=	$(BLDDIR)/obj

# Objects from Makefile.libfdt
LIBFDT_OBJS:=$(addprefix $(OBJDIR)/,$(LIBFDT_OBJS))

include $(SRCDIR)/platforms/$(SOC)/$(SOC).mk

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

compil: mkdir $(OBJDIR)/libfdt.a

mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(OBJDIR) || (echo "- mkdir   $(OBJDIR)" ; mkdir $(OBJDIR))

$(OBJDIR)/libfdt.a: $(LIBFDT_OBJS)
	ar -rc $@ $(LIBFDT_OBJS)
	$(RL) $@

$(OBJDIR)/%.o: %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<

clean:
	rm $(LIBFDT_OBJS) $(OBJDIR)/libfdt.a
