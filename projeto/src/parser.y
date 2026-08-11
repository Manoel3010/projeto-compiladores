/* usado tanto para parser.tab.c quanto para parser.tab.h.
   Precisamos disso porque a %union la' embaixo usa std::string*, No* e Tipo -
   e o lexer.l inclui parser.tab.h sem incluir ast.h por conta propria. */
%code requires {
#include <string>
#include "ast.h"
}

%{
#include <cstdio>
#include "interp.h"

extern int yylex(void);
extern int yylineno;
extern char *yytext;

void yyerror(const char *s);

/* Raiz da AST, preenchida quando 'program' e' reduzido.
   main.cpp le' essa variavel depois que yyparse() retorna 0. */
No *raizAst = nullptr;
%}

%union {
    long         num;
    double       real;
    std::string *str;
    No          *no;
    Tipo         tipoval;
}

/* --- tokens --- */
%token IF ELSE WHILE DO FOR IN STEP BREAK CONTINUE
%token INT FLOAT BOOL CHAR
%token TRUE FALSE
%token <str>  ID
%token <num>  NUM
%token <real> REAL
%token RANGE
%token PIPE
%token OR AND EQ NE LE GE

/* --- tipos dos nao-terminais: qual campo da union cada um usa --- */
%type <no>      program block decls decl stmts stmt expr
%type <tipoval> type

/* --- dangling else: o IF sem ELSE tem precedencia menor que o token ELSE,
       entao o conflito e resolvido sempre pelo shift (ELSE liga ao IF mais
       proximo). --- */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

/* --- precedencia e associatividade dos operadores, da menor para a maior --- */
%right '='
%left PIPE
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
    : block                                      { raizAst = $1; }
    ;

block
    : '{' decls stmts '}'                        { $$ = noBloco($2, $3); }
    ;

decls
    : decls decl                                 { $$ = $1; $$->filhos.push_back($2); }
    | /* vazio */                                { $$ = noLista(); }
    ;

decl
    : type ID ';'                                { $$ = noDecl($1, *$2); delete $2; }
    ;

type
    : INT                                        { $$ = Tipo::INT; }
    | FLOAT                                      { $$ = Tipo::FLOAT; }
    | BOOL                                       { $$ = Tipo::BOOL; }
    | CHAR                                       { $$ = Tipo::CHAR; }
    ;

stmts
    : stmts stmt                                 { $$ = $1; $$->filhos.push_back($2); }
    | /* vazio */                                { $$ = noLista(); }
    ;

stmt
    : expr ';'                                   { $$ = noExprStmt($1); }
    | IF '(' expr ')' stmt                       %prec LOWER_THAN_ELSE
                                                  { $$ = noSe($3, $5); }
    | IF '(' expr ')' stmt ELSE stmt             { $$ = noSeSenao($3, $5, $7); }
    | WHILE '(' expr ')' stmt                    { $$ = noEnquanto($3, $5); }
    | DO stmt WHILE '(' expr ')' ';'             { $$ = noFacaEnquanto($2, $5); }
    /* O corpo do for e um block (exige chaves). Aceitar um stmt qualquer
       tornaria a gramatica ambigua: em "for i in 0..10 -x" o '-' poderia ser
       o menos binario do limite ou o menos unario do corpo, o que produz
       conflitos reduce/reduce que precedencia nao resolve. */
    | FOR ID IN expr RANGE expr block
          { $$ = noPara(*$2, $4, $6, noLiteral(Valor::deInt(1)), $7); delete $2; }
    | FOR ID IN expr RANGE expr STEP expr block
          { $$ = noPara(*$2, $4, $6, $8, $9); delete $2; }
    | BREAK ';'                                  { $$ = noQuebra(); }
    | CONTINUE ';'                               { $$ = noContinua(); }
    | block                                      { $$ = $1; }
    ;

expr
    : ID '=' expr                                { $$ = noAtribuicao(*$1, $3); delete $1; }
    /* Pipeline: "x |> f" == "f(x)" -- x vira o (unico, por enquanto)
       argumento da chamada de f. Encadeado, "x |> f |> g" == g(f(x)),
       porque PIPE e' associativo a esquerda (%left) e tem a precedencia
       mais baixa entre os binarios (so' perde para '='), entao
       "a + b |> f" == "(a + b) |> f". */
    | expr PIPE ID                                { $$ = noChamada(*$3, $1); delete $3; }
    | expr OR expr                               { $$ = noBinario("||", $1, $3); }
    | expr AND expr                              { $$ = noBinario("&&", $1, $3); }
    | expr EQ expr                               { $$ = noBinario("==", $1, $3); }
    | expr NE expr                               { $$ = noBinario("!=", $1, $3); }
    | expr '<' expr                              { $$ = noBinario("<",  $1, $3); }
    | expr '>' expr                              { $$ = noBinario(">",  $1, $3); }
    | expr LE expr                               { $$ = noBinario("<=", $1, $3); }
    | expr GE expr                               { $$ = noBinario(">=", $1, $3); }
    | expr '+' expr                              { $$ = noBinario("+",  $1, $3); }
    | expr '-' expr                              { $$ = noBinario("-",  $1, $3); }
    | expr '*' expr                              { $$ = noBinario("*",  $1, $3); }
    | expr '/' expr                              { $$ = noBinario("/",  $1, $3); }
    | expr '%' expr                              { $$ = noBinario("%",  $1, $3); }
    | '!' expr                                   { $$ = noUnario("!", $2); }
    | '-' expr                                   %prec UMINUS
                                                  { $$ = noUnario("-", $2); }
    | '(' expr ')'                               { $$ = $2; }
    | ID                                         { $$ = noIdent(*$1); delete $1; }
    | NUM                                        { $$ = noLiteral(Valor::deInt($1)); }
    | REAL                                       { $$ = noLiteral(Valor::deFloat($1)); }
    | TRUE                                       { $$ = noLiteral(Valor::deBool(true)); }
    | FALSE                                      { $$ = noLiteral(Valor::deBool(false)); }
    ;

%%

void yyerror(const char *s)
{
    fprintf(stderr, "Erro sintatico na linha %d: %s (proximo a '%s')\n",
            yylineno, s, yytext);
}