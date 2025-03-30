%{

#include "kshell_yacc.h"

#define MAX_NAM_SZ 512 

void count();

%}

filename	        	[a-zA-Z0-9\-@!_]+
alpha					[a-zA-Z]
digit					[0-9]
integer					{digit}+
identifier				{alpha}[a-zA-Z0-9]*

%%


if					count(); return IF;
then				count(); return THEN;
fi					count(); return FI;
else				count(); return ELSE;
while				count(); return WHILE;
do					count(); return DO;
done				count(); return DONE;
ls					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
cat					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
echo				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
export				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
kshell				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
cd					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
pwd					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
top					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
kill				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
kvar				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
su					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return BUILTIN;
"[["                count(); return OBRKTS;
"]]"                count(); return CBRKTS;
"=="				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return EQ;
"!="				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return NEQ;
"<="				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return LEQ;
">="				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return GEQ;
"+"					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return PLUS;
"-"					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return MINUS;
"*"					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return MULT;
"/"				    count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return DIV;
"&&"				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return LAND;
"||"				count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return LOR;
"!"					count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return NOT;
"&"                 count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return AMPERSAND;
"|"				    count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return PIPE;
"="                 count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return ASSING;
";"					count(); return SEMICOLON;
\"(\\.|[^\\"])*\"   count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return STRING;
{filename}			count(); strncpy(yylval.str, yytext, MAX_NAM_SZ) ; return WORD;
{identifier}        count(); strncpy(yylval.str, yytext, MAX_NAM_SZ) ; return IDENTIFIER;
[ \t]+				count(); /* ignore */;
"\n"				count(); return NEWLINE;
.					return (int) yytext[0];


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