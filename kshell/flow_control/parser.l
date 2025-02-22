%{

#include "grammar.h"

%}

authorized_filename		[a-zA-Z0-9_.+-@!]
alpha				[a-zA-Z]
digit				[0-9]
integer				{digit}+
floating			{digit}*\.{digit}{1,6}

%%

'\n'		return NEWLINE;

%%

