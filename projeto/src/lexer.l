%{
#include <stdio.h>
#include "parser.tab.h"
%}

%%

[0-9]+ {
    printf("NUM (%s)\n", yytext);
    return NUM;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    printf("ID (%s)\n", yytext);
    return ID;
}

"+" {
    printf("+\n");
    return '+';
}

\n {
    printf("\\n\n");
    return '\n';
}

[ \t] ;

. {
    printf("DESCONHECIDO: %s\n", yytext);
}

%%

int yywrap() {
    return 1;
}