#ifndef VALUE_H
#define VALUE_H

#include <string>

/* Os quatro tipos que a linguagem conhece. */
enum class Tipo { INT, FLOAT, BOOL, CHAR };

/* Um valor em tempo de execucao: sabe o proprio tipo e guarda o dado.
   E' o que circula pelo interpretador quando uma expressao e' avaliada
   (ex.: "x + 3" produz um Valor do tipo INT). */
struct Valor {
    Tipo tipo;
    union {
        long   i;
        double f;
        bool   b;
        char   c;
    };

    Valor() : tipo(Tipo::INT), i(0) {}

    static Valor deInt(long v)    { Valor r; r.tipo = Tipo::INT;   r.i = v; return r; }
    static Valor deFloat(double v){ Valor r; r.tipo = Tipo::FLOAT; r.f = v; return r; }
    static Valor deBool(bool v)   { Valor r; r.tipo = Tipo::BOOL;  r.b = v; return r; }
    static Valor deChar(char v)   { Valor r; r.tipo = Tipo::CHAR;  r.c = v; return r; }

    /* Conversoes de leitura: pegam o valor guardado, seja qual for o tipo,
       e devolvem como double/long/bool. Usadas para fazer contas entre
       tipos diferentes (ex.: int + float). */
    double comoDouble() const;
    long   comoLong()   const;
    bool   comoBool()   const;

    /* Devolve uma copia deste valor convertida para o tipo alvo.
       Usada na atribuicao: "float media; media = 10;" precisa converter
       o inteiro 10 para 10.0 antes de guardar em 'media'. */
    Valor convertidoPara(Tipo alvo) const;

    /* Representacao textual, usada no dump final das variaveis. */
    std::string paraTexto() const;
};

std::string nomeTipo(Tipo t);

/* Aplica um operador binario (+, -, *, /, %, <, ==, &&, ...) sobre dois
   valores, ja fazendo a promocao de tipo necessaria. 'op' e' o texto do
   operador tal como aparece na gramatica (ex.: "+", "==", "&&"). */
Valor operarBinario(const std::string &op, const Valor &a, const Valor &b);

/* Aplica um operador unario (!, - unario) sobre um valor. */
Valor operarUnario(const std::string &op, const Valor &a);

#endif