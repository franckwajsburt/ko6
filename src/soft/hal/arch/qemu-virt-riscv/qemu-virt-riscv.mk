CC     = riscv64-unknown-elf-gcc#	    compiler
LD     = riscv64-unknown-elf-ld#		linker
OD     = riscv64-unknown-elf-objdump#   desassembler
RL     = riscv64-unknown-elf-ranlib#    index of library

CFLAGS =    -march=rv32g -mabi=ilp32 -mcmodel=medany
CFLAGS +=   -DTH_CONTEXT_SIZE=16#   size of a thread context on this architecture
CFLAGS +=   -g3

LDFLAGS =   -march=rv32g -m elf32lriscv
