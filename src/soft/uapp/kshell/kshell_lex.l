%{

#include "kshell_yacc.h"

%}

authorized_filename		[a-zA-Z0-9_.+-@!]
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
ls					return BUILTIN;
cat					return BUILTIN;
echo				return BUILTIN;
export				return BUILTIN;
kshell				return BUILTIN;
cd					return BUILTIN;
pwd					return BUILTIN;
top					return BUILTIN;
kill				return BUILTIN;
kvar				return BUILTIN;
su					return BUILTIN;
"=="				return BINOP;
"!="				return BINOP;
"<="				return BINOP;
">="				return BINOP;
"+"					yylval.str = yytext; return BINOP;
"-"					yylval.str = yytext; return BINOP;
"*"					yylval.str = yytext; return BINOP;
"\/"				yylval.str = yytext; return BINOP;
{integer}			yylval.num = atoi(yytext); return NUM;
{identifier}		yylval.str = strdup(yytext); return IDENTIFIER;
";"					return SEMICOLON;
[ \t]+				/* ignore */;
"\n"				return NEWLINE;
.					return (int) yytext[0];


%%

