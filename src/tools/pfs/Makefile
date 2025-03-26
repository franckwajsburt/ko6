BLDDIR	= $(ko6)/build#i		 	build directory
SRCDIR	= $(ko6)/src/soft#			source directories
COMDIR	= $(SRCDIR)/common#			user & kernel common files directory
HALDIR	= $(SRCDIR)/hal#			hardware abstraction layer (hal) files directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
OBJDIR	= $(BLDDIR)/kobj#			kernel object files directory
PDFDIR	= $(BLDDIR)/kpdf#			kernel pdf files directory
XLIBDIR = $(ko6)/lib#				external libraries directory


SRCDIR	=
SRCDIR += $(HOME)/ko6/src
INCDIR	= -I. -I$(SRCDIR) -I$(SRCDIR)/soft/ -I$(COMDIR)
CFLAGS  = -D_HOST_
SRC 	= pfs.c pfs.h
include ../Makefile.tool

exec: compil
	pfs

	
# DEPENDENCIES

/home/zheng/ko6/build/kobj/pfs.o: ./pfs.h /usr/include/stdio.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/stdlib.h /usr/include/stdint.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/fcntl.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/features.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/features-time64.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/stdc-predef.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/unistd.h /usr/include/string.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/strings.h
/home/zheng/ko6/build/kobj/pfs.o: /home/zheng/ko6/src/soft/common/list.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/stdio.h /usr/include/stdlib.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/stdint.h /usr/include/fcntl.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/features.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/features-time64.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/stdc-predef.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/unistd.h /usr/include/string.h
/home/zheng/ko6/build/kobj/pfs.o: /usr/include/strings.h
/home/zheng/ko6/build/kobj/pfs.o: /home/zheng/ko6/src/soft/common/list.h
