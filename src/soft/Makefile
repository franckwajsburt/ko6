#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2023-02-24
# | / /(     )/ _ \   	\copyright  2021-2023 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     Makefile
# \author   Franck Wajsburt
# \brief    General Makefile to compile all softs
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

MAKOPT ?= -s#			comment the -s to get command details
SOC    ?= almo1-mips#	defaut SOC name
CPU    ?= mips
VERBOSE?= 0#			verbose mode to print INFO(), BIP(), ASSERT, VAR()
OBJDIR	= ../../build/obj

# Files
# --------------------------------------------------------------------------------------------------

OBJ 	= $(wildcard $(OBJDIR)/*.o)
OBJ 	+= $(OBJDIR)/libfdt.a

# Rules (here they are used such as simple shell scripts)
# --------------------------------------------------------------------------------------------------

.PHONY: help compil tags pdf clean

help:
	@echo ""
	@echo "Usage: make <Target>"
	@echo ""
	@echo "    Target "
	@echo "        compil : compiles all sources"
	@echo "        tags   : generate tags for hypertext navigation (vim)"
	@echo "        pdf    : generate sources.pdf with all source files"
	@echo "        clean  : clean all compiled files"
	@echo ""

compil: tags
	make -C lib/libfdt	$(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	make -C kfs     	$(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	make -C kernel 		$(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	make -C hal 		$(MAKOPT) compil SOC=$(SOC) CPU=$(CPU) VERBOSE=$(VERBOSE)
	make -C ulib   		$(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	make -C uapp   		$(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)

tags:
	@echo "- Create tags for vim"
	@find . -name *.[chS] | xargs ctags 2> tags.log
	@awk '/in files/{print "    "$$5,"and",$$7,$$(NF-2)}' tags.log
	@awk '/in file /{print "    Replication",$$5,$$NF}' tags.log|sed 's/,/:/'

pdf:
	-make -C lib/libfdt $(MAKOPT) pdf
	-make -C kfs    	$(MAKOPT) pdf
	-make -C kernel 	$(MAKOPT) pdf
	-make -C hal 		$(MAKOPT) pdf
	-make -C ulib   	$(MAKOPT) pdf
	-make -C uapp   	$(MAKOPT) pdf

clean:
	-make -C lib/libfdt $(MAKOPT) clean
	-make -C kfs    	$(MAKOPT) clean
	-make -C kernel 	$(MAKOPT) clean
	-make -C hal 		$(MAKOPT) clean
	-make -C ulib   	$(MAKOPT) clean
	-make -C uapp   	$(MAKOPT) clean
	-rm tags tags.log 2> /dev/null
