#include <stdio.h>
#include "kshell_yacc.h"

int main(int argc, char **argv) {
    printf("hello, kshell! :P\n");
    yyparse();

    return 0;
}
