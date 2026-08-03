%{
#include <stdio.h>

int yylex(void);
void yyerror(const char *s);

%}

%token NUM ID

%left '+'

%%

input:
      expr '\n' { YYACCEPT; }
;

expr:
      expr '+' expr
    | NUM
    | ID
;

%%