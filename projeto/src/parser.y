%{
#include <stdio.h>

extern int yylex(void);
extern int yylineno;
extern char *yytext;

void yyerror(const char *s);
%}

/* --- tokens --- */
%token IF ELSE WHILE DO FOR IN STEP BREAK CONTINUE
%token INT FLOAT BOOL CHAR
%token TRUE FALSE
%token ID NUM REAL
%token RANGE
%token OR AND EQ NE LE GE

/* --- dangling else: o IF sem ELSE tem precedencia menor que o token ELSE,
       entao o conflito e resolvido sempre pelo shift (ELSE liga ao IF mais
       proximo). --- */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

/* --- precedencia e associatividade dos operadores, da menor para a maior --- */
%right '='
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right '!' UMINUS

%start program

%%

program
    : block
    ;

block
    : '{' decls stmts '}'
    ;

decls
    : decls decl
    | /* vazio */
    ;

decl
    : type ID ';'
    ;

type
    : INT
    | FLOAT
    | BOOL
    | CHAR
    ;

stmts
    : stmts stmt
    | /* vazio */
    ;

stmt
    : expr ';'
    | IF '(' expr ')' stmt                      %prec LOWER_THAN_ELSE
    | IF '(' expr ')' stmt ELSE stmt
    | WHILE '(' expr ')' stmt
    | DO stmt WHILE '(' expr ')' ';'
    /* O corpo do for e um block (exige chaves). Aceitar um stmt qualquer
       tornaria a gramatica ambigua: em "for i in 0..10 -x" o '-' poderia ser
       o menos binario do limite ou o menos unario do corpo, o que produz
       conflitos reduce/reduce que precedencia nao resolve. */
    | FOR ID IN expr RANGE expr block
    | FOR ID IN expr RANGE expr STEP expr block
    | BREAK ';'
    | CONTINUE ';'
    | block
    ;

expr
    : ID '=' expr
    | expr OR expr
    | expr AND expr
    | expr EQ expr
    | expr NE expr
    | expr '<' expr
    | expr '>' expr
    | expr LE expr
    | expr GE expr
    | expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | '!' expr
    | '-' expr                                  %prec UMINUS
    | '(' expr ')'
    | ID
    | NUM
    | REAL
    | TRUE
    | FALSE
    ;

%%

void yyerror(const char *s)
{
    fprintf(stderr, "Erro sintatico na linha %d: %s (proximo a '%s')\n",
            yylineno, s, yytext);
}
