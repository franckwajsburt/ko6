SRCDIR	= $(ko6)/src/soft/common
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ 
SRC 	= dejavu.c $(SRCDIR)/ht_prob.c	$(SRCDIR)/ctype.c
include ../Makefile.tool

exec: compil
	cat $(SRC) | dejavu
