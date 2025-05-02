SRCDIR	= $(ko6)/src/soft/uapp/kshell
INCDIR	= -I. -I$(SRCDIR) -I$(ko6)/src/soft/common
CFLAGS  = -D_HOST_ -g
CFLAGS += -Wno-unused-result
SRC 	= kshell.c kshell_yacc.c kshell_lex.c
YSRC 	= $(SRCDIR)/kshell_yacc.y
LSRC 	= $(SRCDIR)/kshell_lex.l
include ../Makefile.tool

exec: compil
	kshell
# DEPENDENCIES

/kshell.o: /usr/include/stdio.h kshell_yacc.h
/kshell.o: /home/gwynplaine/ko6/src/soft/uapp/kshell/kshell.h
/kshell.o: /usr/include/stdlib.h /usr/include/stdint.h /usr/include/fcntl.h
/kshell.o: /usr/include/features.h /usr/include/features-time64.h
/kshell.o: /usr/include/stdc-predef.h /usr/include/unistd.h
/kshell.o: /usr/include/string.h /usr/include/strings.h
/kshell.o: /home/gwynplaine/ko6/src/soft/common/htopen.h
/kshell_yacc.o: /usr/include/stdio.h /usr/include/stdlib.h
/kshell_yacc.o: /home/gwynplaine/ko6/src/soft/uapp/kshell/kshell.h
/kshell_yacc.o: /usr/include/stdint.h /usr/include/fcntl.h
/kshell_yacc.o: /usr/include/features.h /usr/include/features-time64.h
/kshell_yacc.o: /usr/include/stdc-predef.h /usr/include/unistd.h
/kshell_yacc.o: /usr/include/string.h /usr/include/strings.h
/kshell_yacc.o: /home/gwynplaine/ko6/src/soft/common/htopen.h kshell_yacc.h
/kshell_yacc.o: /usr/include/limits.h
/kshell_lex.o: /usr/include/stdio.h /usr/include/string.h
/kshell_lex.o: /usr/include/strings.h /usr/include/features.h
/kshell_lex.o: /usr/include/features-time64.h /usr/include/stdc-predef.h
/kshell_lex.o: /home/gwynplaine/ko6/src/soft/common/errno.h
/kshell_lex.o: /usr/include/stdlib.h kshell_yacc.h /usr/include/unistd.h
