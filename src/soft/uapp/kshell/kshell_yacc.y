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
%token BINOP
%token BUILTIN
%token NUM
%token IDENTIFIER


%%

script : no_command top_level no_command
	| no_command
	;

top_level : top_level seq_separator command
	| command
	;

command : stmts separator
	| stmts
	;

stmts : stmts separator stmt
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

if_bloc : IF expr separator THEN top_level FI
	| IF expr separator THEN top_level  ELSE top_level FI
	;

while_bloc : WHILE expr separator DO top_level DONE
	;


expr : 
	  NUM BINOP NUM
	;

built_in : 
	  BUILTIN
	| BUILTIN args
	;

args : NUM args
	| NUM
	;

assignement :
	  IDENTIFIER '=' NUM
	| IDENTIFIER '=' expr
	;

seq_separator :
	  separator
	| seq_separator separator
	;

no_command : seq_separator
	| /* empty */
	;

separator: 
	  NEWLINE 
	| SEMICOLON 
	;

%%

int yyerror(const char * s)
{
	printf("yyerror: %s\n", s);
	return 1;
}


int yywrap() { return 1; }
