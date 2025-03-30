%{

#include <stdio.h>
#include <stdlib.h>
#include "kshell.h"

extern int yylex();
extern int yyerror();
extern int yylex_destroy();

#ifdef KSHELL_DEBUG
int yydebug = 1;
#endif

%}

%code requires { 
	#define MAX_NAM_SZ 1023 
}

%union {
	int num;
	char str[1024];
	struct wordlist *wlist;
	struct stmt *stmt;
}

%token NEWLINE SEMICOLON PIPE
%token IF THEN ELSE FI WHILE DO DONE OBRKTS CBRKTS
%token<str> BINOP STRING AND LAND LOR EQ NEQ LEQ GEQ NOT 
%token<str> PLUS MINUS MULT DIV ASSING AMPERSAND
%token<str> BUILTIN
%token<num> NUM
%token<str> IDENTIFIER
%token<str> WORD
%type<wlist> args
%type<wlist> built_in

%destructor { yylex_destroy(); } script

%%

script : list seq_separator
		{ printf("script!\n"); }
	| list
	| /* empty */
	;

list:
	  list separator top_level { printf("list -> top_level!\n"); }
	| top_level { printf("top_level in list!\n"); }
	;

top_level:
	  pipeline
	| top_level AND linebreak pipeline { printf("top_level in AND PIPELINE\n"); }
	| top_level LOR linebreak pipeline { printf("top_level in LOR PIPELINE\n"); }
	;

pipeline: 
	  pipe_seq
	| NOT pipe_seq
	;

pipe_seq :
	  stmt
	| pipe_seq PIPE seq_separator stmt

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

if_bloc : IF top_level_list THEN top_level_list FI
	{
		printf("if_bloc!\n");
	}
	| IF top_level_list THEN top_level_list  ELSE top_level_list FI
	{
		printf("if_else_bloc\n");
	}
	;

while_bloc : WHILE top_level_list DO top_level_list DONE
	;

top_level_list :
	  maybe_separator top_level maybe_separator
	;

expr : 
	OBRKTS WORD BINOP WORD CBRKTS { 
		printf("binop: %s\n", $3);
		printf("%s %s %s\n", $2, $3, $4);

	  }
	;

var_exp : '$' IDENTIFIER

built_in : 
	  BUILTIN args 
		{
		printf("build-in with args!\n");
		printf("%s :> ", $1);
		wordlist_print($2);
		$$ = wordlist_pushfront($2, $1);
	  	}
	| BUILTIN 
		{
		$$ = make_wordlist($1);
		}
	;

args : WORD args
		{
		$$ = wordlist_pushfront($2, $1);
		}
	| WORD { 
		$$ = make_wordlist($1);
		}
	;

assignement :
	  IDENTIFIER '=' WORD { printf("%s = %s\n", $1, $3); }
	| IDENTIFIER '=' expr
	;

linebreak : newline_list 
	| /* empty*/
	;

newline_list : 
	  NEWLINE
	| newline_list NEWLINE
	;

maybe_separator : seq_separator
	| /* empty */
	;

seq_separator :
	  separator
	| seq_separator separator
	;

separator: 
	  NEWLINE
	| SEMICOLON
	;

%%

extern char yytext[];
extern int column;
extern int line;

int yyerror(const char * s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n at line %d\n", column, "^", column, s, line);
	
	return 1;
}


int yywrap() { return 1; }
