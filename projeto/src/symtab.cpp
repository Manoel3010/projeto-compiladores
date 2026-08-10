#include "symtab.h"
#include <stdexcept>
#include <cstdio>

void TabelaSimbolos::empilharEscopo()
{
    pilha.push_back(Escopo());
}

void TabelaSimbolos::desempilharEscopo()
{
    if (pilha.empty())
        throw std::runtime_error("desempilharEscopo: pilha de escopos vazia");
    pilha.pop_back();
}

void TabelaSimbolos::declarar(const std::string &nome, Tipo tipo)
{
    Escopo &topo = pilha.back();

    if (topo.vars.count(nome))
        throw std::runtime_error("variavel '" + nome + "' ja declarada neste escopo");

    Simbolo s;
    s.tipo = tipo;
    /* Valor inicial "zerado" do tipo declarado. */
    switch (tipo) {
    case Tipo::INT:   s.valor = Valor::deInt(0);     break;
    case Tipo::FLOAT: s.valor = Valor::deFloat(0.0); break;
    case Tipo::BOOL:  s.valor = Valor::deBool(false);break;
    case Tipo::CHAR:  s.valor = Valor::deChar('\0'); break;
    }

    topo.vars[nome] = s;
    topo.ordem.push_back(nome);
}

Valor TabelaSimbolos::obter(const std::string &nome) const
{
    /* Procura do escopo mais interno (fim do vetor) para o mais externo. */
    for (auto it = pilha.rbegin(); it != pilha.rend(); ++it) {
        auto achou = it->vars.find(nome);
        if (achou != it->vars.end())
            return achou->second.valor;
    }
    throw std::runtime_error("variavel '" + nome + "' nao declarada");
}

void TabelaSimbolos::atribuir(const std::string &nome, const Valor &v)
{
    for (auto it = pilha.rbegin(); it != pilha.rend(); ++it) {
        auto achou = it->vars.find(nome);
        if (achou != it->vars.end()) {
            /* Sempre converte para o tipo com que a variavel foi
               declarada (ex.: "float m; m = 10;" guarda 10.0). */
            achou->second.valor = v.convertidoPara(achou->second.tipo);
            return;
        }
    }
    throw std::runtime_error("variavel '" + nome + "' nao declarada");
}

void TabelaSimbolos::imprimirEscopoAtual() const
{
    if (pilha.empty()) return;
    const Escopo &topo = pilha.back();

    if (topo.ordem.empty()) {
        printf("(nenhuma variavel no escopo final)\n");
        return;
    }

    for (const std::string &nome : topo.ordem) {
        const Simbolo &s = topo.vars.at(nome);
        printf("  %-10s %-6s = %s\n",
               nome.c_str(), nomeTipo(s.tipo).c_str(), s.valor.paraTexto().c_str());
    }
}