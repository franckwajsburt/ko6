SRCDIR	= 
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ 
SRC 	= mkdx.c
EXE		= $(wildcard $(BLDDIR)/*/*.x) $(wildcard $(BLDDIR)/*.x) 
include ../Makefile.tool

exec:  # compil
	cd $(BLDDIR); mkdx dskexe.dx $(EXE)
