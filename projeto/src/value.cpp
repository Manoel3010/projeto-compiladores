#include "value.h"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

double Valor::comoDouble() const
{
    switch (tipo) {
    case Tipo::INT:   return (double) i;
    case Tipo::FLOAT: return f;
    case Tipo::BOOL:  return b ? 1.0 : 0.0;
    case Tipo::CHAR:  return (double) c;
    }
    return 0.0;
}

long Valor::comoLong() const
{
    switch (tipo) {
    case Tipo::INT:   return i;
    case Tipo::FLOAT: return (long) f;
    case Tipo::BOOL:  return b ? 1 : 0;
    case Tipo::CHAR:  return (long) c;
    }
    return 0;
}

bool Valor::comoBool() const
{
    switch (tipo) {
    case Tipo::INT:   return i != 0;
    case Tipo::FLOAT: return f != 0.0;
    case Tipo::BOOL:  return b;
    case Tipo::CHAR:  return c != 0;
    }
    return false;
}

Valor Valor::convertidoPara(Tipo alvo) const
{
    switch (alvo) {
    case Tipo::INT:   return Valor::deInt(comoLong());
    case Tipo::FLOAT: return Valor::deFloat(comoDouble());
    case Tipo::BOOL:  return Valor::deBool(comoBool());
    case Tipo::CHAR:  return Valor::deChar((char) comoLong());
    }
    return *this;
}

std::string Valor::paraTexto() const
{
    char buf[64];
    switch (tipo) {
    case Tipo::INT:
        snprintf(buf, sizeof(buf), "%ld", i);
        return buf;
    case Tipo::FLOAT:
        snprintf(buf, sizeof(buf), "%g", f);
        return buf;
    case Tipo::BOOL:
        return b ? "true" : "false";
    case Tipo::CHAR:
        snprintf(buf, sizeof(buf), "'%c'", c);
        return buf;
    }
    return "?";
}

std::string nomeTipo(Tipo t)
{
    switch (t) {
    case Tipo::INT:   return "int";
    case Tipo::FLOAT: return "float";
    case Tipo::BOOL:  return "bool";
    case Tipo::CHAR:  return "char";
    }
    return "?";
}

/* Regra de promocao para operadores aritmeticos e relacionais:
   se QUALQUER um dos dois lados for FLOAT, o resultado e' calculado em
   double. Caso contrario, tudo (INT, BOOL, CHAR) e' tratado como inteiro.
   Isso e' a mesma regra que C usa, so que simplificada. */
static bool algumEhFloat(const Valor &a, const Valor &b)
{
    return a.tipo == Tipo::FLOAT || b.tipo == Tipo::FLOAT;
}

Valor operarBinario(const std::string &op, const Valor &a, const Valor &b)
{
    /* --- operadores logicos: sempre operam sobre bool --- */
    if (op == "&&") return Valor::deBool(a.comoBool() && b.comoBool());
    if (op == "||") return Valor::deBool(a.comoBool() || b.comoBool());

    /* --- comparacoes: resultado e' sempre BOOL --- */
    if (op == "==" || op == "!=" || op == "<" || op == ">" ||
        op == "<=" || op == ">=") {
        bool r;
        if (algumEhFloat(a, b)) {
            double x = a.comoDouble(), y = b.comoDouble();
            if (op == "==") r = (x == y);
            else if (op == "!=") r = (x != y);
            else if (op == "<")  r = (x < y);
            else if (op == ">")  r = (x > y);
            else if (op == "<=") r = (x <= y);
            else                 r = (x >= y);
        } else {
            long x = a.comoLong(), y = b.comoLong();
            if (op == "==") r = (x == y);
            else if (op == "!=") r = (x != y);
            else if (op == "<")  r = (x < y);
            else if (op == ">")  r = (x > y);
            else if (op == "<=") r = (x <= y);
            else                 r = (x >= y);
        }
        return Valor::deBool(r);
    }

    /* --- aritmeticos: INT+INT continua INT, qualquer FLOAT vira FLOAT --- */
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (algumEhFloat(a, b)) {
            double x = a.comoDouble(), y = b.comoDouble();
            if (op == "+") return Valor::deFloat(x + y);
            if (op == "-") return Valor::deFloat(x - y);
            if (op == "*") return Valor::deFloat(x * y);
            if (op == "/") {
                if (y == 0.0) throw std::runtime_error("divisao por zero");
                return Valor::deFloat(x / y);
            }
            /* '%' com float nao existe na linguagem, mas nao deixamos
               o interpretador quebrar por causa disso. */
            throw std::runtime_error("operador '%' nao se aplica a float");
        } else {
            long x = a.comoLong(), y = b.comoLong();
            if (op == "+") return Valor::deInt(x + y);
            if (op == "-") return Valor::deInt(x - y);
            if (op == "*") return Valor::deInt(x * y);
            if (op == "/") {
                if (y == 0) throw std::runtime_error("divisao por zero");
                return Valor::deInt(x / y);
            }
            if (op == "%") {
                if (y == 0) throw std::runtime_error("modulo por zero");
                return Valor::deInt(x % y);
            }
        }
    }

    throw std::runtime_error("operador binario desconhecido: " + op);
}

Valor operarUnario(const std::string &op, const Valor &a)
{
    if (op == "!") return Valor::deBool(!a.comoBool());
    if (op == "-") {
        if (a.tipo == Tipo::FLOAT) return Valor::deFloat(-a.comoDouble());
        return Valor::deInt(-a.comoLong());
    }
    throw std::runtime_error("operador unario desconhecido: " + op);
}