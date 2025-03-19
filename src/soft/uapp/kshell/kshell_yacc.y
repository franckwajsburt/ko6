%{

#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern int yyerror();

#ifdef KSHELL_DEBUG
int yydebug = 1;
#endif

%}

%union {
	int num;
	char * str;
}

%token NEWLINE SEMICOLON
%token IF THEN ELSE FI WHILE DO DONE
%token<str> BINOP
%token<str> BUILTIN
%token<num> NUM
%token<str> IDENTIFIER


%%

script : maybe_separator top_level seq_separator
	| seq_separator
	| /* empty */
	;

top_level : top_level seq_separator stmt
	| stmt
	;

stmt : 
	  simple_stmt
	| compound_stmt
	;

simple_stmt :
	  built_in
	| assignement
	;

compound_stmt :
	  if_bloc
	| while_bloc
	;

if_bloc : IF expr separator THEN top_level FI {
		if_command($2, $3)
	}
	| IF expr separator THEN top_level  ELSE top_level FI
	;

while_bloc : WHILE expr separator DO top_level DONE
	;


expr : 
	  NUM BINOP NUM { 
		printf("binop: %s\n", $2);
		switch ($2[0]) {
			case '+' :
				printf("plus\n");
				break;
			case '*' :
				printf("mult\n");
				break;
			case '/' :
				printf("div\n");
				break;
			case '-' :
				printf("menos\n");
				break;
			
		}
	  }
	;

built_in : 
	  BUILTIN { printf("build-in!\n"); }
	| BUILTIN args { printf("build-in with args!\n"); }
	;

args : NUM args
	| NUM
	;

assignement :
	  IDENTIFIER '=' NUM { printf("%s = %d\n", $1, $3); }
	| IDENTIFIER '=' expr
	;

maybe_separator : seq_separator
	| /* empty */
	;

seq_separator :
	  separator
	| seq_separator separator
	;

separator: 
	  NEWLINE 		{ printf("NEWLINE\n"); }
	| SEMICOLON 	{ printf("SEMICOLON\n"); }
	;

%%

int yyerror(const char * s)
{
	printf("yyerror: %s\n", s);
	return 1;
}


int yywrap() { return 1; }
