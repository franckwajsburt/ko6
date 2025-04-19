SRCDIR	= 
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ 
SRC 	= elfloader.c main.c
include ../Makefile.tool

exec: compil
	@-if [[ -f $(BLDDIR)/barrier/barrier.x ]]; then \
	elfloader  $(BLDDIR)/barrier/barrier.x .text .data ;\
	elfloader  $(BLDDIR)/barrier/barrier.x;\
	fi;\
	if [[ -f   $(BLDDIR)/hello/hello.x ]]; then \
	elfloader  $(BLDDIR)/hello/hello.x .text .data ;\
	elfloader  $(BLDDIR)/hello/hello.x;\
	fi;\
	if [[ -f  $(BLDDIR)/kernel-almo1-mips.x ]]; then \
	elfloader $(BLDDIR)/kernel*.x .ktext .kdata ;\
	elfloader $(BLDDIR)/kernel*.x;\
	fi

