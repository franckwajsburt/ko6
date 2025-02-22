%{

#include <stdio.h>

int yydebug = 1;

%}

%token INTEGER
%token FLOATING
%token ENV_VAR
%token IDENTIFIER
%token IF THEN ELSE FI
%token WHILE DO DONE
%token NEWLINE

%%

top_level : top_level NEWLINE 
	| builtin_call
	| assignment
	| if_block
	| while_block
	| NEWLINE
	;

assignment : IDENTIFIER '=' gen_str
	| IDENTIFIER '=' expression
	;

bin_op : '+' | '-' | '*' | '/'
	| '<' | '>' | LEQ | GEQ
	| EQUALS | DIFFS
	;

if_block : IF expression THEN body FI
	| IF expression THEN body ELSE body FI
	;

while_block : WHILE expression DO body DONE
	;

expression: num_str
	| num_str bin_op num_str
	;

%%

int main()
{
	return 0;
}
