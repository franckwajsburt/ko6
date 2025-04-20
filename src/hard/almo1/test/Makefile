#---------------------------------------------------------------------------------------------------
#  _     ___    __  
# | |__ /'v'\  / /   	\date    	2025-04-20
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/     \licence    https://opensource.org/licenses/MIT
# 
#---------------------------------------------------------------------------------------------------

# Tools and parameters definitions
# --------------------------------------------------------------------------------------------------

NTTYS  ?= 4#                          default number of ttys

CC      = mipsel-unknown-elf-gcc#     compiler
LD      = mipsel-unknown-elf-ld#      linker
OD      = mipsel-unknown-elf-objdump# desassembler
SX      = /dsk/l1/franck/ko6/bin/almo1-mips.x#                    prototype simulator
	   
CFLAGS  = -c#                         stop after compilation, then produce .o
CFLAGS += -Wall -Werror#              gives almost all C warnings and considers them to be errors
CFLAGS += -mips32r2#                  define of MIPS version
CFLAGS += -std=c99#                   define of syntax version of C
CFLAGS += -fno-common#                do not use common sections for non-static vars (only bss)
CFLAGS += -fno-builtin#               do not use builtin functions of gcc (such as strlen)
CFLAGS += -fomit-frame-pointer#       only use of stack pointer ($29)
CFLAGS += -G0#                        do not use global data pointer ($28)
CFLAGS += -O3#                        full optimisation mode of compiler
CFLAGS += -I.#                        directories where include files like <file.h> are located
CFLAGS += -DNTTYS=$(NTTYS)#           #define NTTYS with the number of ttys in the prototype
CFLAGS += -DKO6VER=$(KO6VER)#         last commit
	     
FROM   ?= 0#                          first cycles to trace
NCYC   ?= 10000#                      number of cycles to execute in debug mode

DLOG    = /tmp/debug_$(USER).log

# Rules (here they are used such as simple shell scripts)
# --------------------------------------------------------------------------------------------------

help:
	@echo ""
	@echo "Usage : make <compil|exec|debug|clean>"
	@echo ""
	@echo "        compil  : compile all sources"
	@echo "        exec    : prototype execution"
	@echo "        debug   : Executes and generates execution traces (trace0.s)"
	@echo "        tags    : generate tags for hypertext navigation (vim)"
	@echo "        clean   : clean all compiled files"
	@echo ""

compil:
	$(CC) -o hcpua.o $(CFLAGS) hcpua.S
	@$(OD) -D hcpua.o > hcpua.o.s
	$(LD) -o kernel.x -T kernel.ld hcpua.o
	@$(OD) -D kernel.x > kernel.x.s

exec: compil
	$(SX) -KERNEL kernel.x -NTTYS $(NTTYS) -FBFSIZE 128 -BDFILE roue.raw

debug: clean compil
	$(SX) -KERNEL kernel.x -NTTYS $(NTTYS) -FBFSIZE 128 -BDFILE images.raw\
	      -DEBUG 0 -NCYCLES $(NCYC) > $(DLOG)
	tracelog tags *.x.s $(DLOG)

tags:
	find . -name *.[chS] | xargs ctags

clean:
	@echo clean all but sources
	-@killall xterm soclib-fb 2> /dev/null || true
	-@rm -f tags /tmp/fbf.* *.o* *.x* *~ $(DLOG) trace*.s label*.s xterm* 2> /dev/null || true
