#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2022-06-20
# | / /(     )/ _ \   	\copyright  2021-2022 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     Makefile
# \author   Franck Wajsburt
# \brief    General Makefile to compile all softs
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

MAKOPT ?= -s#						comment the -s to get command details
SOC	   ?= almo1-mips#				defaut SOC name
NTTYS  ?= 2#						default number of ttys
NCPUS  ?= 1#						default number of CPUS
VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()

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
	@echo "        clean  : clean up all compiled files"
	@echo ""

compil: tags
	make -C lib/libfdt	$(MAKOPT) compil SOC=$(SOC) NTTYS=$(NTTYS) NCPUS=$(NCPUS) VERBOSE=$(VERBOSE)
	make -C hal     	$(MAKOPT) compil SOC=$(SOC) NTTYS=$(NTTYS) NCPUS=$(NCPUS) VERBOSE=$(VERBOSE)
	make -C drivers 	$(MAKOPT) compil SOC=$(SOC) NTTYS=$(NTTYS) NCPUS=$(NCPUS) VERBOSE=$(VERBOSE)
	make -C kernel 		$(MAKOPT) compil SOC=$(SOC) NTTYS=$(NTTYS) NCPUS=$(NCPUS) VERBOSE=$(VERBOSE)
	make -C platforms/$(SOC) $(MAKOPT) compil SOC=$(SOC) NTTYS=$(NTTYS) NCPUS=$(NCPUS) VERBOSE=$(VERBOSE)
	make -C ulib   		$(MAKOPT) compil SOC=$(SOC) NTTYS=$(NTTYS) NCPUS=$(NCPUS) VERBOSE=$(VERBOSE)
	make -C uapp   		$(MAKOPT) compil SOC=$(SOC) NTTYS=$(NTTYS) NCPUS=$(NCPUS) VERBOSE=$(VERBOSE) 

tags:
	find . -name *.[chS] | xargs ctags

pdf:
	-make -C lib/libfdt $(MAKOPT) pdf
	-make -C hal    	$(MAKOPT) pdf
	-make -C drivers 	$(MAKOPT) pdf
	-make -C kernel 	$(MAKOPT) pdf
	-make -C platforms/$(SOC) 	$(MAKOPT) pdf SOC=$(SOC)
	-make -C ulib   	$(MAKOPT) pdf
	-make -C uapp   	$(MAKOPT) pdf 

clean:
	-make -C lib/libfdt $(MAKOPT) clean
	-make -C hal    	$(MAKOPT) clean
	-make -C drivers 	$(MAKOPT) clean
	-make -C kernel 	$(MAKOPT) clean
	-make -C platforms/$(SOC) $(MAKOPT) clean
	-make -C kernel 	$(MAKOPT) clean
	-make -C ulib   	$(MAKOPT) clean
	-make -C uapp   	$(MAKOPT) clean
