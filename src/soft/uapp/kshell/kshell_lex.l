%option nounistd

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
#   include <ulib/libc.h>
#   include <ulib/memory.h>
#   include <lexcompat.h>
#endif

#include "kshell_yacc.h"

void count();

%}

filename			[a-zA-Z0-9\-@!?_]+
alpha				[a-zA-Z]
digit				[0-9]
integer				{digit}+
identifier			{alpha}[a-zA-Z0-9_]*
assignment_word                 {identifier}=[^[:space:]]*

%%


if				count(); return IF;
then				count(); return THEN;
fi				count(); return FI;
else				count(); return ELSE;
while				count(); return WHILE;
do				count(); return DO;
done				count(); return DONE;
ls				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
cat				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
echo				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
export				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
kshell				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
cd				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
pwd				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
top				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
kill				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
kvar				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
su				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
"=="				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return EQ;
"!="				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return NEQ;
"<="				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return LEQ;
">="				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return GEQ;
"["				count(); return OBRKT;
"]"				count(); return CBRKT;
"("				count(); return OPAR;
")"				count(); return CPAR;
"+"				count(); return PLUS;
"-"				count(); return MINUS;
"*"				count(); return MULT;
"/"				count(); return DIV;
"&&"				count(); return LAND;
"||"				count(); return LOR;
"<"				count(); return LT;
">"				count(); return GT;
"!"				count(); return NOT;
"&"				count(); return AMPERSAND;
"|"				count(); return PIPE;
"="				count(); return ASSIGN;
";"				count(); return SEMICOLON;
{assignment_word}               count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return ASSIGNMENT_WORD;
{integer}			count(); yylval.num = atoi(yytext); return NUM;
{filename}			count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return WORD;
{identifier}			count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return IDENTIFIER;
[ \t]+				count(); /* ignore */;
"\n"				count(); return NEWLINE;
.				return (int) yytext[0];


%%

int column = 0;
int line   = 1;

void count()
{
    for (int i = 0; yytext[i] != 0; i++) {
        if (yytext[i] == '\n') {
            column = 0;
            line++;
        } else if (yytext[i] == '\t') {
            column += 8;
        } else {
            column++;
        }
    }
}