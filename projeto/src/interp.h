#ifndef INTERP_H
#define INTERP_H

#include "ast.h"

/* Executa o programa representado pela raiz da AST (sempre um no' BLOCO).
   Ao final, imprime o valor de todas as variaveis que sobraram no escopo
   global. Devolve 0 se rodou tudo sem erro, 1 se houve erro em tempo de
   execucao (ex.: divisao por zero, variavel nao declarada). */
int interpretar(No *raiz);

#endif