%{

#include <stdio.h>
#include <stdlib.h>
#include "kshell.h"

extern int yylex();
extern int yyerror();

#ifdef KSHELL_DEBUG
int yydebug = 1;
#endif


%}

%union {
	int num;
	char * str;
	struct wordlist *wlist;
	struct stmt *stmt;
}

%token NEWLINE SEMICOLON
%token IF THEN ELSE FI WHILE DO DONE
%token<str> BINOP
%token<str> BUILTIN
%token<num> NUM
%token<str> IDENTIFIER
%token<str> WORD
%type<wlist> args
%type<wlist> built_in
%type<stmt> top_level
%type<stmt> stmt


%%

script : maybe_separator top_level seq_separator
	| seq_separator
	| /* empty */
	;

top_level : 
	 top_level seq_separator stmt
	{
	}
	| stmt 
	{}
	;

stmt : 
	  simple_stmt 
	| compound_stmt
	;

simple_stmt :
	  built_in {
	  }
	| assignement {}
	;

compound_stmt :
	  if_bloc
	| while_bloc
	;

if_bloc : IF maybe_separator expr separator THEN script FI
	{
		printf("if_bloc!\n");
	}
	| IF maybe_separator expr separator THEN script  ELSE script FI
	;

while_bloc : WHILE expr separator DO top_level DONE
	;

expr : 
	'[' WORD BINOP WORD ']' { 
		printf("binop: %s\n", $3);
		switch ($3[0]) {
			case '+' :
				printf("%s plus %s\n", $2, $4);
				break;
			case '*' :
				printf("%s mult %s\n", $2, $4);
				break;
			case '/' :
				printf("%s div %s\n", $2, $4);
				break;
			case '-' :
				printf("%s menos %s\n", $2, $4);
				break;
			
		}
	  }
	;

built_in : 
	  BUILTIN args 
		{
		printf("build-in with args!\n");
		printf("%s :> ", $1);
		wordlist_print($2);
		$$ = wordlist_pushfront($2, $1);
		free($1);
	  	}
	| BUILTIN 
		{
		$$ = make_wordlist($1);
		free($1);
		}
	;

args : WORD args
		{
		$$ = wordlist_pushfront($2, $1);
		free($1);
		}
	| WORD { 
		$$ = make_wordlist($1);
		free($1);
		}
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
