%option yylineno
%option noyywrap

%x COMENTARIO

%{
#include <stdio.h>
#include "parser.tab.h"

/* Contador de erros lexicos, consultado pelo main para definir o codigo de saida. */
int erros_lexicos = 0;
%}

DIGITO  [0-9]
LETRA   [a-zA-Z_]

%%

    /* --- espacos em branco (yylineno conta as quebras automaticamente) --- */
[ \t\r\n]+              { }

    /* --- comentarios --- */
"//".*                  { }

"/*"                    { BEGIN(COMENTARIO); }
<COMENTARIO>"*/"        { BEGIN(INITIAL); }
<COMENTARIO>\n          { }
<COMENTARIO>.           { }
<COMENTARIO><<EOF>>     {
                            fprintf(stderr, "Erro lexico na linha %d: comentario /* nao fechado\n", yylineno);
                            erros_lexicos++;
                            BEGIN(INITIAL);
                            return 0;
                        }

    /* --- palavras-chave (antes de ID) --- */
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"do"                    { return DO; }
"for"                   { return FOR; }
"in"                    { return IN; }
"step"                  { return STEP; }
"break"                 { return BREAK; }
"continue"              { return CONTINUE; }
"true"                  { return TRUE; }
"false"                 { return FALSE; }
"int"                   { return INT; }
"float"                 { return FLOAT; }
"bool"                  { return BOOL; }
"char"                  { return CHAR; }

    /* --- identificadores --- */
{LETRA}({LETRA}|{DIGITO})*  { return ID; }

    /* --- literais numericos: REAL vem antes de NUM ---
       Em "0..10" a regra REAL nao fecha (depois do ponto vem outro ponto),
       o Flex faz backtrack e casa NUM(0), depois RANGE, depois NUM(10). */
{DIGITO}+"."{DIGITO}+   { return REAL; }
{DIGITO}+               { return NUM; }

    /* --- operador de range --- */
".."                    { return RANGE; }

    /* --- operadores de dois caracteres (antes dos de um) --- */
"&&"                    { return AND; }
"||"                    { return OR; }
"=="                    { return EQ; }
"!="                    { return NE; }
"<="                    { return LE; }
">="                    { return GE; }

    /* --- operadores de um caractere --- */
"!"                     { return '!'; }
"<"                     { return '<'; }
">"                     { return '>'; }
"="                     { return '='; }
"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }
"%"                     { return '%'; }

    /* --- delimitadores --- */
"("                     { return '('; }
")"                     { return ')'; }
"{"                     { return '{'; }
"}"                     { return '}'; }
"["                     { return '['; }
"]"                     { return ']'; }
";"                     { return ';'; }
","                     { return ','; }

    /* --- qualquer outra coisa --- */
.                       {
                            fprintf(stderr, "Erro lexico na linha %d: caractere desconhecido '%s'\n",
                                    yylineno, yytext);
                            erros_lexicos++;
                        }

%%
