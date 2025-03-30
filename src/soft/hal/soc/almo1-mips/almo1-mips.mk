#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /      \date       2025-03-30
# | / /(     )/ _ \     \copyright  2021 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     hal/soc/almo1-mips/almo1-mips.mk
# \author   Franck Wajsburt
# \brief    MIPS specific tools and parameters
#
#--------------------------------------------------------------------------------------------------

CPU		= mips

CC		= mipsel-unknown-elf-gcc#		compiler
LD		= mipsel-unknown-elf-ld#		linker
OD		= mipsel-unknown-elf-objdump#	desassembler
RL		= mipsel-unknown-elf-ranlib#	index of library
RL		= touch#     					if no ranlib required
        
CFLAGS	=  -mips32r2#					define of MIPS version
CFLAGS	+= -G0#							do not use global data pointer ($28)
CFLAGS	+= -DTH_CONTEXT_SIZE=13#		size of a thread context on this architecture
