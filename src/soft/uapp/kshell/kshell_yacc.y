%{


#ifdef _HOST_
#   include <stdio.h>
#   include <stdlib.h>
#   include <stdint.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <string.h>
#   include <sys/types.h>
#   include <stdint.h>
#   define MALLOC(s) malloc(s)
#   define FREE(s) free(s)
#   define P(fmt,var) fprintf(stderr, #var" : "fmt, var)
#   define RETURN(e,c,m,...) if(c){fprintf(stderr,"Error "m"\n");__VA_ARGS__;return e;}
#   define OPENR(f) open (f, O_RDONLY)
#   define OPENW(f) open (f, O_WRONLY | O_CREAT | O_TRUNC, 0644)
#   define PRINT(fmt,...) printf(fmt, ##__VA_ARGS__)
#else
#   define MALLOC(s) malloc(s)
#   define FREE(s) free(s)
#   define P(fmt,var) 
#   define RETURN(e,c,m,...) if(c){kprintf("Error "m"\n");__VA_ARGS__;return e;}
#   define OPENR(f) open (f)
#   define OPENW(f)
#   define PRINT(fmt,...)
#   include <common/cstd.h>    
#	include <ulib/libc.h>
#	include <ulib/memory.h>
#endif


#include "kshell.h"

extern int yylex();
extern int yyerror();
extern int yylex_destroy();

extern stmt_s *curr;
extern stmt_s *chkpnt;
extern expr_s *aux;

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
	struct expr *expr;
}

%token NEWLINE SEMICOLON PIPE AMPERSAND
%token IF THEN ELSE FI WHILE DO DONE
%token OBRKT CBRKT OPAR CPAR
%token PLUS MINUS MULT DIV
%token LT GT LEQ GEQ EQ NEQ
%token LAND LOR NOT ASSIGN
%token<str> BUILTIN
%token<num> NUM
%token<str> ASSIGNMENT_WORD
%token<str> IDENTIFIER
%token<str> WORD
%type<stmt> simple_stmt compound_stmt stmt if_bloc while_bloc
%type<stmt> top_level top_level_list list
%type<wlist> words
%type<wlist> built_in
%type<expr> factor mult_expr add_expr bool_expr expr rel_expr arithmetic_expansion


%destructor { 
	yylex_destroy();
	/* \TODO hash table destruction */

} script

%%

script : linebreak list linebreak
	  {
		PRINT("script!\n");
	  }
	| linebreak
	;

list:
	  list separator top_level 
	  {
		PRINT("list -> top_level!\n");
		stmt_print($3);
		kshell_stmt_execute($3);
		//$$ = $3;
		stmt_destroy($3);
		kshell_print_env();
	  }
	| top_level
	  { 
		PRINT("top_level in list!\n");
		stmt_print($1);
		kshell_stmt_execute($1);
		//$$ = $1;
		stmt_destroy($1);
		kshell_print_env();
	  }
	;

top_level:
	  pipeline
	| top_level LAND linebreak pipeline
	  {
		PRINT("top_level in AND PIPELINE\n");
	  }
	| top_level LOR linebreak pipeline 
	  {
		PRINT("top_level in LOR PIPELINE\n");
	  }
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
	   {
		$$ = $1;
	   }
	| compound_stmt
	  {
		$$ = $1;
	  }
	;

simple_stmt :
	  built_in
	  {
		$$ = stmt_create();
		stmt_set_simple($$, $1, BUILT_IN_TYPE);
	  }
	| ASSIGNMENT_WORD
	  {
		$$ = stmt_create();
		int i;

		for (i = 0; i < strlen($1); i++) {
			if ($1[i] == '=') { // todo: handle escape chars
				$1[i] = 0;
				break;
			}
		}

		wordlist_s *w = make_wordlist($1+i+1);
		w = wordlist_pushfront(w, $1);
		stmt_set_simple($$, w, ENV_ASSIGN_TYPE);
	  }
	| arithmetic_expansion
	  {
		// this part is just for testing purposes
		$$ = stmt_create();
		stmt_set_expr($$, $1);
	  }
	;

compound_stmt :
	  if_bloc
	| while_bloc
	;

if_bloc : IF top_level_list THEN top_level_list FI
	  {
		$$ = stmt_create();
		stmt_set_if_stmt($$, $2, $4, NULL);
	  }
	| IF top_level_list THEN top_level_list ELSE top_level_list FI
	  {
		$$ = stmt_create();
		stmt_set_if_stmt($$, $2, $4, $6);
	  }
	;

while_bloc : WHILE top_level_list DO top_level_list DONE
	  {
		$$ = stmt_create();
		stmt_set_while_stmt($$, $2, $4);
		chkpnt = $$;
	  }
	;

top_level_list :
	  top_level_list top_level separator
	  {
		stmt_set_next($1, $2);
		$$ = $1;
	  }
	| maybe_separator top_level separator
	  {
		$$ = $2;
	  }
	;

arithmetic_expansion : 
	'$' OBRKT expr CBRKT
	  { 
		$$ = $3;
	  }
	;

expr :
	  IDENTIFIER ASSIGN expr
	  {
		$$ = expr_create();
		expr_s *id_expr = expr_create();
		expr_set_word(id_expr, $1);
		expr_set_op($$, ASSIGN_OP, id_expr, $3);
	  }
	| bool_expr
	  {
		$$ = $1;
	  }
	;

bool_expr :
	  bool_expr LAND rel_expr
	  {
		$$ = expr_create();
		expr_set_op($$, AND_OP, $1, $3);
	  }
	|  bool_expr LOR rel_expr
	  {
		$$ = expr_create();
		expr_set_op($$, OR_OP, $1, $3);
	  }
	| NOT rel_expr
	  {
		$$ = expr_create();
		expr_set_op($$, NOT_OP, $2, NULL);
	  }
	| rel_expr
	  {
		$$ = $1;
	  }
	;

rel_expr :
	  add_expr LT add_expr
	  {
		$$ = expr_create();
		expr_set_op($$, LT_OP, $1, $3);
	  }
	| add_expr GT add_expr
	  {
		$$ = expr_create();
		expr_set_op($$, GT_OP, $1, $3);
	  }
	| add_expr LEQ add_expr
	  {
		$$ = expr_create();
		expr_set_op($$, LEQ_OP, $1, $3);
	  }
	| add_expr GEQ add_expr
	  {
		$$ = expr_create();
		expr_set_op($$, GEQ_OP, $1, $3);
	  }
	| add_expr EQ add_expr
	  {
		$$ = expr_create();
		expr_set_op($$, EQ_OP, $1, $3);
	  }
	| add_expr NEQ add_expr
	  {
		$$ = expr_create();
		expr_set_op($$, NEQ_OP, $1, $3);
	  }
	| add_expr
	  {
		$$ = $1;
	  }
	;

add_expr :
	  add_expr PLUS mult_expr
	  {
		$$ = expr_create();
		expr_set_op($$, PLUS_OP, $1, $3);
	  }
	| add_expr MINUS mult_expr
	  {
		$$ = expr_create();
		expr_set_op($$, MINUS_OP, $1, $3);
	  }
	| mult_expr
	  {
		$$ = $1;
	  }
	;

mult_expr :
	  mult_expr MULT factor
	  {
		$$ = expr_create();
		expr_set_op($$, MULT_OP, $1, $3);
	  }
	| mult_expr DIV factor
	  {
		$$ = expr_create();
		expr_set_op($$, DIV_OP, $1, $3);
	  }
	| factor
	  {
		$$ = $1;
	  }
	;

factor :
	  NUM
	  {
		$$ = expr_create();
		expr_set_int($$, $1);
	  }
	| WORD
	  {
		$$ = expr_create();
		expr_set_word($$, $1);
	  }
	| OPAR expr CPAR
	  {
		$$ = $2;
	  }
	;

var_exp : '$' IDENTIFIER

built_in : 
	  BUILTIN words 
	  {
		$$ = wordlist_pushfront($2, $1);
	  }
	| BUILTIN 
	  {
		$$ = make_wordlist($1);
	  }
	;

words : WORD words
	  {
		$$ = wordlist_pushfront($2, $1);
	  }
	| WORD
	  { 
		$$ = make_wordlist($1);
	  }
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
	//fflush(stdout);
	PRINT("\n%*s\n%*s at line %d\n", column, "^", column, s, line);
	
	return 1;
}


int yywrap() { return 1; }
