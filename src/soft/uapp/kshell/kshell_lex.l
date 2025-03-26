%{

#include "kshell_yacc.h"

%}

filename	        	[a-zA-Z0-9_.+-@!]+
alpha					[a-zA-Z]
digit					[0-9]
integer					{digit}+
identifier				{alpha}[a-zA-Z0-9]*

%%


if					return IF;
then				return THEN;
fi					return FI;
else				return ELSE;
while				return WHILE;
do					return DO;
done				return DONE;
ls					yylval.str = strdup(yytext); return BUILTIN;
cat					yylval.str = strdup(yytext); return BUILTIN;
echo				yylval.str = strdup(yytext); return BUILTIN;
export				yylval.str = strdup(yytext); return BUILTIN;
kshell				yylval.str = strdup(yytext); return BUILTIN;
cd					yylval.str = strdup(yytext); return BUILTIN;
pwd					yylval.str = strdup(yytext); return BUILTIN;
top					yylval.str = strdup(yytext); return BUILTIN;
kill				yylval.str = strdup(yytext); return BUILTIN;
kvar				yylval.str = strdup(yytext); return BUILTIN;
su					yylval.str = strdup(yytext); return BUILTIN;
"=="				yylval.str = strdup(yytext); return BINOP;
"!="				yylval.str = strdup(yytext); return BINOP;
"<="				yylval.str = strdup(yytext); return BINOP;
">="				yylval.str = strdup(yytext); return BINOP;
"+"					yylval.str = strdup(yytext); return BINOP;
"-"					yylval.str = strdup(yytext); return BINOP;
"*"					yylval.str = strdup(yytext); return BINOP;
"\/"				yylval.str = strdup(yytext); return BINOP;
{filename}			yylval.str = strdup(yytext); return WORD;
";"					return SEMICOLON;
[ \t]+				/* ignore */;
"\n"				return NEWLINE;
.					return (int) yytext[0];


%%

