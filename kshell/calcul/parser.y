%{

#include <stdio.h>
#include <stdlib.h>

#ifdef KSHELL_DEBUG
int yydebug = 1;
#endif

%}


%token NEWLINE

%%

top_level : top_level NEWLINE 
	  | NEWLINE
	  ;

%%

int yyerror(const char * s)
{
	perror( "error");
	return 0;
}

int yywrap() { return 1; }

void main(void) { yyparse(); }
