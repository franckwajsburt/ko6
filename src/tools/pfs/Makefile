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

	
