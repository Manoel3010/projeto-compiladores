#include "interp.h"
#include "symtab.h"
#include <cstdio>
#include <stdexcept>

/* Sinal que um comando devolve para quem o executou, usado para
   implementar break/continue: um comando comum devolve NORMAL; um
   'break'/'continue' devolve QUEBRA/CONTINUA, e essa informacao sobe
   pela recursao ate' o laco (for/while/do-while) mais proximo, que e'
   quem sabe o que fazer com ela. Um 'if' ou um bloco, por exemplo, so'
   repassam o sinal para cima sem tratar. */
enum class Sinal { NORMAL, QUEBRA, CONTINUA };

/* RAII: garante que desempilharEscopo() sempre roda, mesmo se uma
   excecao for lancada no meio do bloco (erro em tempo de execucao). */
struct GuardaEscopo {
    TabelaSimbolos &tabela;
    explicit GuardaEscopo(TabelaSimbolos &t) : tabela(t) { tabela.empilharEscopo(); }
    ~GuardaEscopo() { tabela.desempilharEscopo(); }
};

static Valor avaliar(No *n, TabelaSimbolos &tabela);
static Sinal executarComando(No *n, TabelaSimbolos &tabela);
static Sinal executarBloco(No *bloco, TabelaSimbolos &tabela);

/* ---------------------------------------------------------------------
   Expressoes: sempre devolvem um Valor.
   --------------------------------------------------------------------- */
static Valor avaliar(No *n, TabelaSimbolos &tabela)
{
    switch (n->tipo) {
    case NoTipo::LITERAL:
        return n->valor;

    case NoTipo::IDENT:
        return tabela.obter(n->texto);

    case NoTipo::ATRIBUICAO: {
        Valor v = avaliar(n->filhos[0], tabela);
        tabela.atribuir(n->texto, v);
        /* O valor de uma atribuicao e' o proprio valor atribuido - e' o
           que permite "x = y = 3;" (associatividade a direita, ja
           declarada em parser.y com %right '='). */
        return v;
    }

    case NoTipo::BINARIO: {
        Valor esq = avaliar(n->filhos[0], tabela);
        Valor dir = avaliar(n->filhos[1], tabela);
        return operarBinario(n->texto, esq, dir);
    }

    case NoTipo::UNARIO: {
        Valor a = avaliar(n->filhos[0], tabela);
        return operarUnario(n->texto, a);
    }

    case NoTipo::CHAMADA: {
        /* Chegou aqui via pipeline: "x |> f" virou noChamada("f", x).
           A linguagem nao tem funcoes definidas pelo usuario (nao ha 'fn'),
           entao so' reconhecemos um conjunto pequeno de funcoes embutidas.
           Qualquer outro nome e' um erro em tempo de execucao, tratado
           igual a divisao por zero (try/catch ja existente em
           interpretar()) -- nao e' um crash. */
        Valor arg = avaliar(n->filhos[0], tabela);

        if (n->texto == "print") {
            printf("%s\n", arg.paraTexto().c_str());
            /* print "devolve" o proprio valor que recebeu, para poder
               continuar um pipeline depois dele (ex.: x |> print |> f). */
            return arg;
        }

        throw std::runtime_error(
            "funcao '" + n->texto + "' nao existe "
            "(a linguagem ainda nao tem funcoes definidas pelo usuario; "
            "a unica funcao embutida disponivel para o pipeline hoje e' 'print')");
    }

    default:
        throw std::runtime_error("no' inesperado em avaliar()");
    }
}

/* ---------------------------------------------------------------------
   Bloco "{ }": empilha um escopo, declara as variaveis locais, executa
   os comandos em ordem. Se algum comando devolver QUEBRA/CONTINUA, o
   restante do bloco e' abortado e o sinal sobe para quem chamou.
   --------------------------------------------------------------------- */
static Sinal executarBloco(No *bloco, TabelaSimbolos &tabela)
{
    GuardaEscopo guarda(tabela);

    for (No *filho : bloco->filhos) {
        if (filho->tipo == NoTipo::DECL) {
            tabela.declarar(filho->texto, filho->tipoDecl);
            continue;
        }
        Sinal s = executarComando(filho, tabela);
        if (s != Sinal::NORMAL)
            return s;
    }
    return Sinal::NORMAL;
}

/* ---------------------------------------------------------------------
   Comandos.
   --------------------------------------------------------------------- */
static Sinal executarComando(No *n, TabelaSimbolos &tabela)
{
    switch (n->tipo) {

    case NoTipo::BLOCO:
        return executarBloco(n, tabela);

    case NoTipo::EXPR_STMT:
        avaliar(n->filhos[0], tabela);
        return Sinal::NORMAL;

    case NoTipo::SE:
        if (avaliar(n->filhos[0], tabela).comoBool())
            return executarComando(n->filhos[1], tabela);
        return Sinal::NORMAL;

    case NoTipo::SE_SENAO:
        if (avaliar(n->filhos[0], tabela).comoBool())
            return executarComando(n->filhos[1], tabela);
        else
            return executarComando(n->filhos[2], tabela);

    case NoTipo::ENQUANTO:
        while (avaliar(n->filhos[0], tabela).comoBool()) {
            Sinal s = executarComando(n->filhos[1], tabela);
            if (s == Sinal::QUEBRA) break;
            /* CONTINUA nao precisa de tratamento especial aqui: so' vai
               direto para a proxima checagem da condicao, que e' o que
               o loop 'while' do C++ ja faz sozinho. */
        }
        return Sinal::NORMAL;

    case NoTipo::FACA_ENQUANTO:
        do {
            Sinal s = executarComando(n->filhos[0], tabela);
            if (s == Sinal::QUEBRA) break;
        } while (avaliar(n->filhos[1], tabela).comoBool());
        return Sinal::NORMAL;

    case NoTipo::PARA: {
        Valor inicio = avaliar(n->filhos[0], tabela);
        Valor fim    = avaliar(n->filhos[1], tabela);
        Valor passo  = avaliar(n->filhos[2], tabela);
        No   *corpo  = n->filhos[3];

        /* A variavel do 'for' e' float se qualquer uma das tres pontas
           for float; senao e' int. Ela vive num escopo proprio, que
           envolve o escopo do corpo (assim ela some quando o laco
           termina, mas o corpo ainda pode ve-la). */
        bool ehFloat = (inicio.tipo == Tipo::FLOAT || fim.tipo == Tipo::FLOAT ||
                        passo.tipo == Tipo::FLOAT);
        Tipo tipoVar = ehFloat ? Tipo::FLOAT : Tipo::INT;

        double atual   = inicio.comoDouble();
        double alvo    = fim.comoDouble();
        double delta   = passo.comoDouble();

        if (delta == 0.0)
            throw std::runtime_error("'step' do for nao pode ser zero");

        GuardaEscopo guardaVar(tabela);
        tabela.declarar(n->texto, tipoVar);

        while ((delta > 0 && atual < alvo) || (delta < 0 && atual > alvo)) {
            tabela.atribuir(n->texto, ehFloat ? Valor::deFloat(atual) : Valor::deInt((long) atual));

            Sinal s = executarComando(corpo, tabela);
            if (s == Sinal::QUEBRA) break;

            atual += delta;
        }
        return Sinal::NORMAL;
    }

    case NoTipo::QUEBRA:
        return Sinal::QUEBRA;

    case NoTipo::CONTINUA:
        return Sinal::CONTINUA;

    default:
        throw std::runtime_error("comando inesperado em executarComando()");
    }
}

/* ---------------------------------------------------------------------
   Ponto de entrada chamado pelo main.cpp.
   --------------------------------------------------------------------- */
int interpretar(No *raiz)
{
    TabelaSimbolos tabela;
    tabela.empilharEscopo();

    try {
        for (No *filho : raiz->filhos) {
            if (filho->tipo == NoTipo::DECL) {
                tabela.declarar(filho->texto, filho->tipoDecl);
            } else {
                /* No escopo global nao existe laco para tratar o sinal, entao
                   um 'break'/'continue' que chega aqui esta' fora de contexto.
                   Antes dessa checagem o sinal era descartado em silencio e a
                   execucao simplesmente continuava no comando seguinte. */
                Sinal s = executarComando(filho, tabela);
                if (s == Sinal::QUEBRA)
                    throw std::runtime_error("'break' fora de um laco");
                if (s == Sinal::CONTINUA)
                    throw std::runtime_error("'continue' fora de um laco");
            }
        }
    } catch (const std::exception &e) {
        fprintf(stderr, "Erro em tempo de execucao: %s\n", e.what());
        tabela.desempilharEscopo();
        return 1;
    }

    printf("Execucao concluida. Valores finais das variaveis:\n");
    tabela.imprimirEscopoAtual();
    tabela.desempilharEscopo();
    return 0;
}