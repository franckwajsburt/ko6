SRCDIR	= $(ko6)/src/tools/mkdx
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ 
SRC 	= mkdx.c
EXE		= $(wildcard $(BLDDIR)/*/*.x) $(wildcard $(BLDDIR)/*.x) 
TEST	= $(wildcard $(SRCDIR)/*)
include ../Makefile.tool

exec:  # compil
	echo $(TEST)
	cd $(BLDDIR); mkdx dskexe.dx $(EXE) $(TEST)
