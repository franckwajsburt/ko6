#-------------------------------------------------------------------------------------------------*\
#  _     ___    __
# | |__ /'v'\  / /      \date		2023-02-24
# | / /(     )/ _ \     \copyright	2021-2023 Sorbonne University
# |_\_\ x___x \___/			https://opensource.org/licenses/MIT
#
# \file     uapp/Makefile
# \author   Franck Wajsburt
# \brief    Makefile to produce all the application executables
#
#-------------------------------------------------------------------------------------------------*/

APPS   ?= hello barrier
VERBOSE?= 0#				verbose level
MAKOPT ?= #-s#				comment the -s to get command details
ACTIONS = clean compil pdf#		all possible actions

# Rules (here they are used such as simple shell scripts)
# --------------------------------------------------------------------------------------------------

.PHONY: $(ACTIONS)

help:
	@echo ""
	@echo "Usage : make <$(ACTIONS)>"
	@echo ""
	@echo "        compil  : compile all apps (defined is APPS parameter)"
	@echo "                  "$(APPS)
	@echo "        clean   : clean up all compiled files for all APPS"
	@echo "        pdf     : generate pdf with all source files for all APPS"
	@echo ""

# Macro-generations of all the rules :
#  | clean : hello/clean ... and so on for all APPS
#  | hello/clean :; make clean -C hello $(MAKOPT) ...
#  | ... as many rules as APPS
#  | compil : hello/compil ...  and so on for all APPS
#  | ... as many rules as APPS
# --------------------------------------------------------------------------------------------------

$(foreach a,$(ACTIONS),\
	$(eval $(a): $(addsuffix /$(a), $(APPS)))\
	$(foreach d,$(APPS),\
		$(eval $(d)/$(a):;\
			@$(MAKE) $(a) -C $(d) $(MAKOPT) VERBOSE=$(VERBOSE)\
		)\
	)\
)
