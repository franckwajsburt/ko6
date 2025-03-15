SRCDIR	= $(ko6)/src/soft/uapp/kshell
INCDIR	= -I. -I$(SRCDIR)
CFLAGS  = -D_HOST_ 
SRC 	= kshell.c kshell_yacc.c kshell_lex.c
YSRC 	= $(SRCDIR)/kshell_yacc.y
LSRC 	= $(SRCDIR)/kshell_lex.l
include ../Makefile.tool

exec: compil
	kshell
# DEPENDENCIES

/kshell.o: /usr/include/stdio.h kshell_yacc.h
/kshell_yacc.o: /usr/include/stdio.h /usr/include/stdlib.h kshell_yacc.h
/kshell_yacc.o: /usr/include/limits.h
/kshell_lex.o: /usr/include/stdio.h /usr/include/string.h
/kshell_lex.o: /usr/include/errno.h /usr/include/features.h
/kshell_lex.o: /usr/include/features-time64.h /usr/include/stdc-predef.h
/kshell_lex.o: /usr/include/stdlib.h kshell_yacc.h /usr/include/unistd.h
