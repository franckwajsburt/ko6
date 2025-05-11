#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /      \date       2025-02-24
# | / /(     )/ _ \     \copyright  2021 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     tools/Makefile.tool
# \author   Franck Wajsburt
# \brief    Compile Linux Tools file
# 			This Makefile must be included in tool's Makefile, see dejavu/Makefile for details
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

V      ?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()
BLDDIR	= $(ko6)/build#		build directory
BINDIR	= $(ko6)/bin#		bin directory
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
PDF 	= $(PDFDIR)/$(CURDIR).pdf#	the pdf file has the name of directory
BIN     = $(BINDIR)/$(CURDIR)#		the binary file has the name of directory
YACC	= bison
LEX		= flex
BISONVER= $(shell bison -V|head -1| cut -d" " -f4)
ifeq ($(BISONVER),3.7.4)
	BISONDEF = "--defines"
else
	BISONDEF = "-H"
endif
	

# Sources
# --------------------------------------------------------------------------------------------------

SRC	   ?= "source files"#			must be defined in all app Makefiles

# Targets
# --------------------------------------------------------------------------------------------------

SRCC    = $(filter %.c,$(SRC))

# Tools
# --------------------------------------------------------------------------------------------------

CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -Wno-unused-function#     ignore the unused warning
CFLAGS += -Wno-unused-result# 		ignore the unused result from fonction
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += $(INCDIR)#				directories where include files like <file.h> are located
CFLAGS += -DVERBOSE=$(V)#			verbose if 1, can be toggled with #include <debug_{on,off}.h> 

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
	@echo "        exec    : test the program"
	@echo ""

compil: depend mkdir $(BIN)

mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)

pdf:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(PDFDIR) || (echo "- mkdir   $(PDFDIR)" ; mkdir $(PDFDIR))
	a2ps -T4 -2 -M A4 -A fill -o - -l100 Makefile $(SRC) 2> $(PDF).log |\
	ps2pdf -sPAPERSIZE=a4 - $(PDF);\
	echo "- create $(PDF)";\
	sed 's/^/  - /;s/\].*/\]/' $(PDF).log

clean:
	@echo "- clean $(notdir $(BIN)) and related build files"
	-rm *~ $(OBJ) $(OBJDS) .*.bin *.bin $(BIN)* $(PDF) $(PDF).log \
			*_yacc.c *_lex.c *_yacc.h \
			*.pdf *.dot 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Rules with automatic variables to produce the executable
# --------------------------------------------------------------------------------------------------

$(BIN) : $(SRC)
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $^

%.c : $(SRCDIR)/%.y
	@echo "- bison   --> "$(notdir $@)
	echo "_$(BISONVER)_"
	$(YACC) $(BISONDEF) $< -o $(notdir $@)
#	$(CC) -D_HOST_ -I$(SRCDIR) -o $(notdir ${@:.c=.o}) $(notdir ${^:.y=.c})

%.c : $(SRCDIR)/%.l
	@echo "- flex    --> "$(notdir $@)
	$(LEX) -o $(notdir $@) $<
#	$(CC) -D_HOST_ -I$(SRCDIR) -o $(notdir ${@:.c=.o}) $(notdir ${^:.l=.c})

# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRC) 2> /dev/null
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/$(SOCDIR)/:/:' Makefile > Makefile.bak 
	mv Makefile.bak Makefile
