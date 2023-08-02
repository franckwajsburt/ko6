CC     = mipsel-unknown-elf-gcc#		compiler
LD     = mipsel-unknown-elf-ld#			linker
OD     = mipsel-unknown-elf-objdump#    desassembler
RL     = mipsel-unknown-elf-ranlib#     index of library
RL     = echo#     						if no ranlib required

CFLAGS =    -mips32r2#				define of MIPS version
CFLAGS +=   -G0#				    do not use global data pointer ($28)
CFLAGS +=   -DTH_CONTEXT_SIZE=13#   size of a thread context on this architecture
