#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2025-02-26
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     Makefile
# \author   Franck Wajsburt
# \brief    General Makefile to compile all softs
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

MAKOPT ?= -s#				comment the -s to get command details
SOC    ?= almo1-mips#		defaut SOC name
CPU    ?= mips
VERBOSE?= 0#				verbose mode to print INFO(), BIP(), ASSERT, VAR()
BLDDIR	= $(ko6)/build#		build directory
OBJDIR	= $(BLDDIR)/kobj#	kernel object files directory

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
	-make -C external/libfdt $(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	-make -C fs/kfs          $(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	-make -C kernel          $(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	-make -C hal             $(MAKOPT) compil SOC=$(SOC) CPU=$(CPU) VERBOSE=$(VERBOSE)
	-make -C ulib            $(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	-make -C uapp            $(MAKOPT) compil SOC=$(SOC) VERBOSE=$(VERBOSE)
	mkdx $(BLDDIR)/dskexe.img $(BLDDIR)/*/*.x $(BLDDIR)/*.x 

tags:
	@echo "- Create tags for vim"
	@find . -name *.[chS] | xargs ctags 

pdf:
	@-make -C external/libfdt $(MAKOPT) pdf
	@-make -C fs/kfs  	  	 $(MAKOPT) pdf
	@-make -C kernel 		 $(MAKOPT) pdf
	@-make -C hal 			 $(MAKOPT) pdf
	@-make -C ulib   		 $(MAKOPT) pdf
	@-make -C uapp   		 $(MAKOPT) pdf

clean:
	@-make -C external/libfdt $(MAKOPT) clean
	@-make -C fs/kfs     $(MAKOPT) clean
	@-make -C kernel 		 $(MAKOPT) clean
	@-make -C hal 			 $(MAKOPT) clean
	@-make -C ulib   		 $(MAKOPT) clean
	@-make -C uapp   		 $(MAKOPT) clean
	@-rm tags 2> /dev/null
