SRCDIR	= $(ko6)/src/soft/common
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_
SRC 	= $(CURDIR).c $(SRCDIR)/radix.c	
include ../Makefile.tool
D	?= 1
exec: compil
	$(CURDIR) $(D)
	dot -Tpdf -otest.pdf test.dot
	dot -Tpdf -otest2.pdf test2.dot
