SRCDIR	= 
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ 
SRC 	= elfloader.c
include ../Makefile.tool

exec: compil
	@-if [[ -f $(ko6)/build/hello.x ]]; then \
	elfloader $(ko6)/build/hello.x .text .data ;\
	fi;\
	if [[ -f $(ko6)/build/kernel-almo1-mips.x ]]; then \
	elfloader $(ko6)/build/kernel*.x .ktext .kdata ;\
	fi

