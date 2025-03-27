SRCDIR	= 
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ 
SRC 	= elfloader.c
include ../Makefile.tool

exec: compil
	@-if [[ -f $(BLDDIR)/hello.x ]]; then \
	elfloader  $(BLDDIR)/hello.x .text .data ;\
	elfloader  $(BLDDIR)/hello.x;\
	fi;\
	if [[ -f  $(BLDDIR)/kernel-almo1-mips.x ]]; then \
	elfloader $(BLDDIR)/kernel*.x .ktext .kdata ;\
	elfloader $(BLDDIR)/kernel*.x;\
	fi

