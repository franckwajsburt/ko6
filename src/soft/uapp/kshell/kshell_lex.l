%{

#include "kshell_yacc.h"

%}

authorized_filename		[a-zA-Z0-9_.+-@!]
alpha				[a-zA-Z]
digit				[0-9]
integer				{digit}+
identifier			{alpha}({alpha}{digit})*

%%


if				printf("IF\n"); return IF;
then				printf("THEN\n"); return THEN;
fi				return FI;
else				return ELSE;
while				return WHILE;
do				return DO;
done				return DONE;
ls				return BUILTIN;
cat				return BUILTIN;
echo				return BUILTIN;
export				return BUILTIN;
kshell				return BUILTIN;
cd				return BUILTIN;
pwd				return BUILTIN;
top				return BUILTIN;
kill				return BUILTIN;
kvar				return BUILTIN;
su				return BUILTIN;
"=="				return BINOP;
"!="				return BINOP;
"<="				return BINOP;
">="				return BINOP;
'+'				return BINOP;
'-'				return BINOP;
'*'				return BINOP;
'\/'				return BINOP;
{integer}			/*yylval.num = atoi(yytext);*/ printf("NUM\n"); return NUM;
{identifier}			/*yylval.str = strdup(yytext);*/printf("IDENTIFIER\n"); return IDENTIFIER;
";"				printf("SEMICOLON\n"); return SEMICOLON;
[ \t]+				/* ignore */;
'\n'				printf("NEWLINE\n"); return NEWLINE;
.				return (int) yytext[0];


%%

