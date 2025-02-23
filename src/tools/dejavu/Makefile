CFLAGS	= -D_HOST_ -Wall -I. -I$(ko6)/src/soft/common 
SRC		= dejavu.c $(ko6)/src/soft/common/ht_prob.c
BIN		= dejavu
F		= $(SRC)

help:
	@echo -e "\nHash table validation\n"
	@echo -e "Usage:\tmake <target> [F=<file>]"
	@echo -e "target:"
	@echo -e "\thelp:\tDefault target"
	@echo -e "\tcompil:\tOnly compile"
	@echo -e "\texec:\tCompile then execute"
	@echo -e "\tclean:\tDelete all except sources\n"

compil:	$(BIN)

exec: compil
	cat $(F) | ./$(BIN)

$(BIN) : $(SRC)
	$(CC) -o $(BIN) $(CFLAGS) $(SRC)

clean:
	rm *~ $(BIN) 2> /dev/null || true
