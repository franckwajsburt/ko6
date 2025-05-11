SRCDIR	= $(ko6)/src/soft/uapp/kshell/
KO6DIR = $(ko6)/src/soft
INCDIR	= -I. -I$(SRCDIR) -I$(KO6DIR)
CFLAGS  = -D_HOST_ -g
CFLAGS += -Wno-unused-result
SRCFIL	= kshell.c varenv.c wordlist.c stmt.c
SRC		= $(addprefix $(SRCDIR), $(SRCFIL))
SRC    += $(KO6DIR)/common/htopen.c kshell_yacc.c kshell_lex.c
YSRC 	= $(SRCDIR)/kshell_yacc.y
LSRC 	= $(SRCDIR)/kshell_lex.l
include ../Makefile.tool

exec: compil
	kshell
