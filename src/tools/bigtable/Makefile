SRCDIR	= $(ko6)/src/soft/common
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ -g
SRC 	= $(CURDIR).c $(SRCDIR)/htopen.c	$(SRCDIR)/ctype.c
include ../Makefile.tool

exec: compil
	$(CURDIR) 512 80
