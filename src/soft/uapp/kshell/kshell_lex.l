%{

#include "kshell_yacc.h"    

void count();

%}

filename			[a-zA-Z0-9\-@!_]+
alpha				[a-zA-Z]
digit				[0-9]
integer				{digit}+
identifier			{alpha}[a-zA-Z0-9_]*
assignment_word                 {identifier}=[^[:space:]]+

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
{identifier}			count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return IDENTIFIER;
{filename}			count(); strncpy(yylval.str, yytext, MAX_NAM_SZ); return WORD;
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