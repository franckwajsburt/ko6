#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2025-04-20
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/     \licence    https://opensource.org/licenses/MIT
#
# \file     Makefile
# \author   Franck Wajsburt
# \brief    General Makefile to compile all ko6's code : kernel; libraries and user applications
#
#--------------------------------------------------------------------------------------------------

# General settings, directories and tools specific to the SOC 
# --------------------------------------------------------------------------------------------------

VERBOSE?= 0#				verbose mode to print INFO(), BIP(), ASSERT, VAR()
MAKOPT ?= -s#				comment the -s to get command details

SOC    ?= almo1-mips#		default SOC name
BLDDIR	= $(ko6)/build#		build directory

# Simple shell scripts
# --------------------------------------------------------------------------------------------------

.PHONY: help compil tags pdf clean

help:
	@echo "$(CPU) $(SOC)"
	@echo "Usage: make <Target>"
	@echo ""
	@echo "    Target "
	@echo "        compil : compiles all sources"
	@echo "        tags   : generate tags for hypertext navigation (vim)"
	@echo "        pdf    : generate sources.pdf with all source files"
	@echo "        clean  : clean all compiled files"
	@echo ""

compil: tags
	-make -C external/libfdt  $(MAKOPT) compil VERBOSE=$(VERBOSE) SOC=$(SOC)
	-make -C fs/kfs           $(MAKOPT) compil VERBOSE=$(VERBOSE) SOC=$(SOC)
	-make -C kernel           $(MAKOPT) compil VERBOSE=$(VERBOSE) SOC=$(SOC)
	-make -C hal              $(MAKOPT) compil VERBOSE=$(VERBOSE) SOC=$(SOC)
	-make -C ulib             $(MAKOPT) compil VERBOSE=$(VERBOSE) SOC=$(SOC)
	-make -C uapp             $(MAKOPT) compil VERBOSE=$(VERBOSE) SOC=$(SOC)
	-mkdx $(BLDDIR)/dskexe.dx $(BLDDIR)/*/*.x

tags:
	@echo "- Create tags for vim"
	@find . -name *.[chS] | grep -v riscv | xargs ctags 

pdf:
	@-make -C external/libfdt $(MAKOPT) pdf
	@-make -C fs/kfs  	  	  $(MAKOPT) pdf
	@-make -C kernel 		  $(MAKOPT) pdf
	@-make -C hal 			  $(MAKOPT) pdf
	@-make -C ulib   		  $(MAKOPT) pdf
	@-make -C uapp   		  $(MAKOPT) pdf
                              
clean:
	@-make -C external/libfdt $(MAKOPT) clean
	@-make -C fs/kfs     	  $(MAKOPT) clean
	@-make -C kernel 		  $(MAKOPT) clean
	@-make -C hal 			  $(MAKOPT) clean
	@-make -C ulib   		  $(MAKOPT) clean
	@-make -C uapp   		  $(MAKOPT) clean
	@-rm tags 2> /dev/null
