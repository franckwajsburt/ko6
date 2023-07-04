#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date      	2022-07-03
# | / /(     )/ _ \   	\copyright  2021-2022 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     kernel/Makefile
# \author   Franck Wajsburt
# \brief    Makefile to produce the kernel executable: kernel.x
#
#--------------------------------------------------------------------------------------------------

# Parameters
# --------------------------------------------------------------------------------------------------

SOC	   ?= almo1#					defaut SOC name
NTTYS  ?= 1#						default number of ttys
NCPUS  ?= 1#						default number of CPUS
VERBOSE?= 0#						verbose mode to print INFO(), BIP(), ASSERT, VAR()
BLDDIR	= ../../../build#			build directory
COMDIR	= ../common#				user & kernel common files directory
LIBDIR	= .#						library directory (where is ldscript)
HALDIR	= ../hal#					hardware abstraction layer (hal) files directory
SOCDIR	= $(HALDIR)/$(SOC)#			current soc hardware abstraction layer (hal) files directory
CURDIR 	= $(notdir $(shell pwd))#	name of curent directory
OBJDIR	= $(BLDDIR)/obj#			object files directory
PDFDIR	= $(BLDDIR)/pdf#			pdf files directory
LIBDIR  = ../../lib#				external libraries directory
# Sources
# --------------------------------------------------------------------------------------------------

SRC     = $(COMDIR)/debug_on.h $(COMDIR)/debug_off.h
SRC    += $(COMDIR)/syscalls.h ksyscalls.c
SRC    += $(COMDIR)/usermem.h $(COMDIR)/list.h
SRC    += $(COMDIR)/errno.h $(COMDIR)/errno.c $(COMDIR)/esc_code.h
SRC    += $(HALDIR)/harch.h $(SOCDIR)/harch.c
SRC    += $(HALDIR)/hcpu.h $(SOCDIR)/hcpu_soc.h $(SOCDIR)/hcpua.S $(SOCDIR)/hcpuc.c $(SOCDIR)/dtb.S
SRC	   += $(COMDIR)/cstd.c $(COMDIR)/cstd.h
SRC    += kmemory.c kmemory.h
SRC    += klibc.c klibc.h
SRC    += kthread.c kthread.h
SRC    += kinit.c
SRC    += ksynchro.c ksynchro.h
LDSCRIPT= $(SOCDIR)/kernel.ld#		ldscript is hal dependent

# Targets
# --------------------------------------------------------------------------------------------------

SRCS    = $(filter %.S,$(SRC))
SRCC    = $(filter %.c,$(SRC))
OBJS    = $(subst .S,.o,$(foreach src,$(SRCS),$(notdir $(src))))
OBJC    = $(subst .c,.o,$(foreach src,$(SRCC),$(notdir $(src))))
OBJ     = $(addprefix $(OBJDIR)/,$(OBJS) $(OBJC))
OBJ 	+= $(OBJDIR)/libfdt.a

OBJDS   = $(addsuffix .s,$(OBJ))
PDF 	= $(PDFDIR)/$(CURDIR).pdf#  the pdf file has the name of directory
BIN     = $(BLDDIR)/$(CURDIR).x

# Tools
# --------------------------------------------------------------------------------------------------

CC     = mipsel-unknown-elf-gcc#	compiler
LD     = mipsel-unknown-elf-ld#		linker
OD     = mipsel-unknown-elf-objdump#desassembler

CFLAGS  = -c#						stop after compilation, then produce .o
CFLAGS += -Wall -Werror#			gives almost all C warnings and considers them to be errors
CFLAGS += -mips32r2#				define of MIPS version
CFLAGS += -std=c99#					define of syntax version of C
CFLAGS += -fno-common#				do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#				do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#		only use of stack pointer ($29)
CFLAGS += -G0#						do not use global data pointer ($28)
CFLAGS += -O3#						full optimisation mode of compiler
CFLAGS += -I. -I$(COMDIR)#			directories where include files like <file.h> are located
CFLAGS += -I$(HALDIR) -I$(SOCDIR)#	directories where include files like <file.h> are located
CFLAGS += -I$(LIBDIR)/libfdt#		include external libraries (specifically libfdt.h)
CFLAGS += -DNTTYS=$(NTTYS)#			#define NTTYS with the number of ttys in the prototype
CFLAGS += -DNCPUS=$(NCPUS)#			#define NCPUS with the number of cpus in the prototype
CFLAGS += -DVERBOSE=$(VERBOSE)#		verbose if 1, can be toggled with #include <debug_{on,off}.h>

# Rules (here they are used such as simple shell scripts)
# --------------------------------------------------------------------------------------------------

.PHONY: help compil mkdir pdf clean depend

help:
	@echo ""
	@echo "Usage : make <compil|clean|pdf>"
	@echo ""
	@echo "        compil  : compile all sources"
	@echo "        clean   : clean up all compiled files"
	@echo "        pdf     : generate $(PDF) with all source files"
	@echo ""

compil: depend mkdir mkdtb $(BIN)

mkdir:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(OBJDIR) || (echo "- mkdir   $(OBJDIR)" ; mkdir $(OBJDIR))

mkdtb:
	dtc -I dts -O dtb -o $(SOC).dtb $(SOCDIR)/$(SOC).dts

pdf:
	test -d $(BLDDIR) || mkdir $(BLDDIR)
	test -d $(PDFDIR) || (echo "- mkdir   $(PDFDIR)" ; mkdir $(PDFDIR))
	a2ps -T4 -2 -M A4 -A fill -o - -l100 Makefile $(SRC) 2> $(PDF).log |\
	ps2pdf -sPAPERSIZE=a4 - $(PDF);\
	echo "- create $(PDF)";\
	sed 's/^/  - /;s/\].*/\]/' $(PDF).log

clean:
	@echo "- clean up $(BIN) and related build files"
	-rm *~ *.dtb $(OBJ) $(OBJDS) $(BIN)* $(PDF) $(PDF).log 2> /dev/null || true
	awk '/^# DEPENDENCIES/{stop=1}(!stop){print}' Makefile > Makefile.bak
	mv Makefile.bak Makefile

# Rules with automatic variables to produce the executable
# --------------------------------------------------------------------------------------------------

$(BIN) : $(LDSCRIPT) $(OBJ)
	@echo "- link    --> "$(notdir $@)
	$(LD) -o $@ -T $^
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : %.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(SOCDIR)/%.S
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(COMDIR)/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

$(OBJDIR)/%.o : $(SOCDIR)/%.c
	@echo "- compil  --> "$(notdir $@)
	$(CC) -o $@ $(CFLAGS) $<
	$(OD) -D $@ > $@.s

# makedepend analyzes the source files to determine automatically what are the dependencies
# of the object files on the source files (see https://linux.die.net/man/1/makedepend for details)
depend :
	@echo "- makedepend for $(CURDIR)"
	makedepend -- $(CFLAGS) -- -D__DEPEND__ -s"# DEPENDENCIES" -p$(OBJDIR)/ $(SRCC) $(SRCS)
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/$(SOCDIR)/:/:' Makefile > Makefile.bak
	/usr/bin/sed '/^# DEPENDENCIES/,$$s:/$(COMDIR)/:/:' Makefile.bak > Makefile
	rm Makefile.bak

# --------------------------------------------------------------------------------------------------
# DEPENDENCIES

../../../build/obj/ksyscalls.o: ./klibc.h ../common/debug_off.h
../../../build/obj/ksyscalls.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/ksyscalls.o: ../common/list.h ../hal/hcpu.h
../../../build/obj/ksyscalls.o: ../hal/almo1/hcpu_soc.h ../hal/harch.h
../../../build/obj/ksyscalls.o: ../common/usermem.h ./kmemory.h ./kthread.h
../../../build/obj/ksyscalls.o: ../common/esc_code.h ../common/errno.h
../../../build/obj/ksyscalls.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/errno.o: ../common/errno.h
../../../build/obj/harch.o: ./klibc.h ../common/debug_off.h
../../../build/obj/harch.o: ../common/debug_on.h
../../../build/obj/harch.o: ../common/cstd.h ../common/list.h
../../../build/obj/harch.o: ../hal/hcpu.h
../../../build/obj/harch.o: ../hal/almo1/hcpu_soc.h
../../../build/obj/harch.o: ../hal/harch.h ../common/usermem.h
../../../build/obj/harch.o: ./kmemory.h ./kthread.h
../../../build/obj/harch.o: ../common/esc_code.h
../../../build/obj/harch.o: ../common/errno.h
../../../build/obj/harch.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/hcpuc.o: ./klibc.h ../common/debug_off.h
../../../build/obj/hcpuc.o: ../common/debug_on.h
../../../build/obj/hcpuc.o: ../common/cstd.h ../common/list.h
../../../build/obj/hcpuc.o: ../hal/hcpu.h
../../../build/obj/hcpuc.o: ../hal/almo1/hcpu_soc.h
../../../build/obj/hcpuc.o: ../hal/harch.h ../common/usermem.h
../../../build/obj/hcpuc.o: ./kmemory.h ./kthread.h
../../../build/obj/hcpuc.o: ../common/esc_code.h
../../../build/obj/hcpuc.o: ../common/errno.h
../../../build/obj/hcpuc.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/cstd.o: ../common/cstd.h
../../../build/obj/kmemory.o: ./klibc.h ../common/debug_off.h
../../../build/obj/kmemory.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/kmemory.o: ../common/list.h ../hal/hcpu.h
../../../build/obj/kmemory.o: ../hal/almo1/hcpu_soc.h ../hal/harch.h
../../../build/obj/kmemory.o: ../common/usermem.h ./kmemory.h ./kthread.h
../../../build/obj/kmemory.o: ../common/esc_code.h ../common/errno.h
../../../build/obj/kmemory.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/klibc.o: ./klibc.h ../common/debug_off.h
../../../build/obj/klibc.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/klibc.o: ../common/list.h ../hal/hcpu.h
../../../build/obj/klibc.o: ../hal/almo1/hcpu_soc.h ../hal/harch.h
../../../build/obj/klibc.o: ../common/usermem.h ./kmemory.h ./kthread.h
../../../build/obj/klibc.o: ../common/esc_code.h ../common/errno.h
../../../build/obj/klibc.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/kthread.o: ./klibc.h ../common/debug_off.h
../../../build/obj/kthread.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/kthread.o: ../common/list.h ../hal/hcpu.h
../../../build/obj/kthread.o: ../hal/almo1/hcpu_soc.h ../hal/harch.h
../../../build/obj/kthread.o: ../common/usermem.h ./kmemory.h ./kthread.h
../../../build/obj/kthread.o: ../common/esc_code.h ../common/errno.h
../../../build/obj/kthread.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/kinit.o: ../../lib/libfdt/libfdt.h
../../../build/obj/kinit.o: ../../lib/libfdt/libfdt_env.h ../common/cstd.h
../../../build/obj/kinit.o: ../../lib/libfdt/fdt.h ./klibc.h
../../../build/obj/kinit.o: ../common/debug_off.h ../common/debug_on.h
../../../build/obj/kinit.o: ../common/list.h ../hal/hcpu.h
../../../build/obj/kinit.o: ../hal/almo1/hcpu_soc.h ../hal/harch.h
../../../build/obj/kinit.o: ../common/usermem.h ./kmemory.h ./kthread.h
../../../build/obj/kinit.o: ../common/esc_code.h ../common/errno.h
../../../build/obj/kinit.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/ksynchro.o: ./klibc.h ../common/debug_off.h
../../../build/obj/ksynchro.o: ../common/debug_on.h ../common/cstd.h
../../../build/obj/ksynchro.o: ../common/list.h ../hal/hcpu.h
../../../build/obj/ksynchro.o: ../hal/almo1/hcpu_soc.h ../hal/harch.h
../../../build/obj/ksynchro.o: ../common/usermem.h ./kmemory.h ./kthread.h
../../../build/obj/ksynchro.o: ../common/esc_code.h ../common/errno.h
../../../build/obj/ksynchro.o: ../common/syscalls.h ./ksynchro.h
../../../build/obj/hcpua.o: ../hal/hcpu.h
../../../build/obj/hcpua.o: ../hal/almo1/hcpu_soc.h
../../../build/obj/hcpua.o: ../common/syscalls.h
