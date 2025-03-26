SRCDIR	= $(ko6)/src/soft/uapp/kshell
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ -g
CFLAGS += -Wno-unused-result
SRC 	= kshell.c kshell_yacc.c kshell_lex.c
YSRC 	= $(SRCDIR)/kshell_yacc.y
LSRC 	= $(SRCDIR)/kshell_lex.l
include ../Makefile.tool

exec: compil
	kshell
