#---------------------------------------------------------------------------------------------------
#  _     ___    __
# | |__ /'v'\  / /   	\date       2025-02-19
# | / /(     )/ _ \   	\copyright  2021 Sorbonne University
# |_\_\ x___x \___/                 https://opensource.org/licenses/MIT
#
# \file     Makefile
# \author   Franck Wajsburt
# \brief    compile kfstools to build and access to kfsdisk
#
#--------------------------------------------------------------------------------------------------

CFLAGS 	= -Wall -g -I$(ko6)/src/soft/fs/kfs -DHOST
BIN		= $(ko6)/bin/kfstools
SRC		= kfstools.c $(ko6)/src/soft/fs/kfs/kfs.c

$(BIN) : $(SRC)
	$(CC) -o $(BIN) $(CFLAGS) $(SRC)

clean:
	@echo Remove binaries
	@rm -r $(BIN) *.o *~ dummy* 2>/dev/null||true

