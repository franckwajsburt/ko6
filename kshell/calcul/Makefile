CC = gcc
FLAGS = -g -Wall

parser: grammar.o lexer.o
	$(CC) $(FLAGS) $^ -o $@

grammar.o: parser.y
	bison -d -v $^ -o $(@:.o=.c) --debug
	$(CC) -c grammar.c -o $@ $(FLAGS)

lexer.o: parser.l
	flex -d $^
	$(CC) -c lex.yy.c -o $@ $(FLAGS)


