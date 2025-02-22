CC	= gcc
FLAGS	= -g -Wall

parser: kshell_y.o kshell_l.o
	${CC} -o $< $@ ${FLAGS}

kshell_y.o:
	bison -d -v kshell_y.y
	$(CC) -c $(@:.o=.tab.c) -o $< ${FLAGS}

kshell_l.o:
	flex -d kshell_l.l
	$(CC) -c $(@:.o=.yy.c) -o $< ${FLAGS}

clean:
	rm *.o *.tab.c *.tab.h *.output *.yy.c
