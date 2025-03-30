#-------------------------------------------------------------------------------------------------*\
#  _     ___    __
# | |__ /'v'\  / /      \date		2025-03-30
# | / /(     )/ _ \     \copyright	2021 Sorbonne University
# |_\_\ x___x \___/					https://opensource.org/licenses/MIT
#
# \file     uapp/Makefile
# \author   Franck Wajsburt
# \brief    Makefile for compiling all executable user applications 
#
#-------------------------------------------------------------------------------------------------*/

APPS    = hello barrier dejavu kshell

VERBOSE ?= 0#		verbose level
MAKOPT  ?= #-s#		comment the -s to get command details

# Simple shell script, mainly generated
# --------------------------------------------------------------------------------------------------
# clean         : hello/clean barrier/clean ... and so on for all APPS
# hello/clean   :; make clean -C hello $(MAKOPT) ...
# barrier/clean :; make clean -C hello $(MAKOPT) ...
# ... as many rules as APPS
# 
# compil 	   	: hello/compil barrier/compil ...  and so on for all APPS
# hello/compil  :; make compil -C hello $(MAKOPT) ...
# ... as many rules as APPS
#
# pdf			: hello/pdf	 barrier/pdf ...  and so on for all APPS
# ... 
# --------------------------------------------------------------------------------------------------

ACTIONS	= clean compil pdf#		all possible actions

.PHONY: $(ACTIONS)

help:
	@echo ""
	@echo "Usage : make <$(ACTIONS)>"
	@echo ""
	@echo "        compil  : compile all apps (defined is APPS parameter)"
	@echo "                  "$(APPS)
	@echo "        clean   : clean all compiled files for all APPS"
	@echo "        pdf     : generate pdf with all source files for all APPS"
	@echo ""

$(foreach a,$(ACTIONS),\
	$(eval $(a): $(addsuffix /$(a), $(APPS)))\
	$(foreach d,$(APPS),\
		$(eval $(d)/$(a):;\
			@$(MAKE) $(a) -C $(d) $(MAKOPT) VERBOSE=$(VERBOSE)\
		)\
	)\
)
