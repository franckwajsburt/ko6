# --------------------------------------------------------------------------------------------------
# generic makefile to compile the soclib hardware simulator
# --------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

CPU	   ?= mips
SOC     = $(notdir $(shell pwd))# 	 SOC name is the name of the current directory
BLDDIR	= ../../bin
TARGET  = $(BLDDIR)/$(SOC)-$(CPU).x# name of the generated simulator
REPOS   = $(BLDDIR)/systemcass#		 soclib object files (see soclib.conf)

# Rules (here they are used such as simple shell scripts)
# --------------------------------------------------------------------------------------------------

.PHONY: dep all clean cleanall mkdir

help    :
	@echo ""
	@echo "Usage : make <dep|all|clean|cleanall> [-s]"
	@echo ""
	@echo "        dep     : compile the $(SOC) simulator if required"
	@echo "        clean   : delete compiled files but the $(SOC) simulator"
	@echo "        cleanall: clean then delete $(SOC) simulator"
	@echo "        all     : do \"make cleanall dep\""
	@echo "        -s      : silent mode"
	@echo ""

dep: mkdir $(TARGET)

all: cleanall dep

mkdir:
	test -d $(BLDDIR) || (echo "- mkdir   $(BLDDIR)" ; mkdir $(BLDDIR))

clean   :
	@echo "- clean all binary files of the [$(SOC)] simulator"
	soclib-cc -x -X -P -p $(SOC).desc
	-rm -r $(REPOS) 2> /dev/null || true
	-rm $(TARGET) 2> /dev/null || true

$(TARGET): $(SOC).desc $(SOC)_top.cpp $(SOC)_config.h
	export nbcores=$(shell lscpu -p|grep -v "#" |wc -l);\
	echo "- parallel compilation (on $$nbcores cores) of the $(SOC) simulator";\
	soclib-cc -j$$nbcores -P -t mysystemcass64static -p $< -o $@
	rm mysystemcass64static_caba*.o
	strip $(TARGET)
