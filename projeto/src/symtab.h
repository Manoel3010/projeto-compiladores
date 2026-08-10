#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <vector>
#include <unordered_map>
#include "value.h"

struct Simbolo {
    Tipo  tipo;
    Valor valor;
};

/* Tabela de simbolos com escopo aninhado.
   Cada bloco "{ }" que o interpretador executa empilha um escopo novo
   com empilharEscopo() e o desempilha com desempilharEscopo() ao sair.
   Buscar uma variavel (obter/atribuir) procura do escopo mais interno
   para o mais externo - e' assim que uma variavel do bloco de fora
   continua visivel dentro de um "if" ou "while" aninhado. */
class TabelaSimbolos {
public:
    void empilharEscopo();
    void desempilharEscopo();

    /* Cria a variavel no escopo ATUAL (o do topo da pilha).
       Lanca erro se a variavel ja existir nesse MESMO escopo
       (redeclaracao), mas permite "esconder" uma variavel de fora. */
    void declarar(const std::string &nome, Tipo tipo);

    Valor obter(const std::string &nome) const;
    void  atribuir(const std::string &nome, const Valor &v);

    /* Imprime nome/tipo/valor de todas as variaveis do escopo do topo
       da pilha (usado para o dump final, chamado antes de desempilhar
       o escopo global). */
    void imprimirEscopoAtual() const;

private:
    /* 'vars' guarda os dados; 'ordem' guarda os nomes na ordem em que
       foram declarados, so' para o dump final sair na ordem do codigo
       (unordered_map, como o nome diz, nao garante nenhuma ordem). */
    struct Escopo {
        std::unordered_map<std::string, Simbolo> vars;
        std::vector<std::string> ordem;
    };
    std::vector<Escopo> pilha;
};

#endif