#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include "value.h"

/* Cada categoria possivel de no' da arvore. Um unico "No" generico serve
   para tudo (comandos e expressoes); o campo 'tipo' diz o que ele
   representa, e o interpretador decide o que fazer olhando para ele.
   Essa abordagem (um struct so, com uma "tag") e' mais simples de
   implementar em um projeto didatico do que criar uma classe C++
   diferente para cada tipo de comando/expressao. */
enum class NoTipo {
    LISTA,       /* container generico, usado so' durante a construcao (decls/stmts) */

    BLOCO,       /* filhos = decls (NoTipo::DECL) seguidos de comandos, nessa ordem */
    DECL,        /* declaracao de variavel: texto = nome, tipoDecl = tipo */

    EXPR_STMT,   /* comando que e' so' uma expressao seguida de ';' */
    SE,          /* if sem else:   filhos = [condicao, corpo] */
    SE_SENAO,    /* if com else:   filhos = [condicao, corpo_se, corpo_senao] */
    ENQUANTO,    /* while:         filhos = [condicao, corpo] */
    FACA_ENQUANTO, /* do-while:    filhos = [corpo, condicao] */
    PARA,        /* for i in a..b step c { }: texto = nome da variavel,
                     filhos = [inicio, fim, passo, corpo] (passo = 1 se omitido) */
    QUEBRA,      /* break */
    CONTINUA,    /* continue */

    ATRIBUICAO,  /* texto = nome da variavel, filhos = [expr do lado direito] */
    BINARIO,     /* texto = operador ("+", "==", "&&", ...), filhos = [esq, dir] */
    UNARIO,      /* texto = operador ("!", "-"),              filhos = [operando] */
    IDENT,       /* texto = nome da variavel */
    LITERAL      /* valor = o proprio valor literal (NUM/REAL/TRUE/FALSE) */
};

struct No {
    NoTipo tipo;
    std::vector<No*> filhos;

    std::string texto;   /* nome de variavel, ou operador */
    Valor       valor;   /* usado em LITERAL */
    Tipo        tipoDecl; /* usado em DECL */

    explicit No(NoTipo t) : tipo(t), tipoDecl(Tipo::INT) {}
};

/* Funcoes de conveniencia para montar nos dentro das acoes semanticas
   do parser.y, sem precisar repetir "new No(...)" com os campos na mao. */
No* noLista();
No* noBloco(No *decls, No *stmts);
No* noDecl(Tipo tipo, const std::string &nome);
No* noExprStmt(No *expr);
No* noSe(No *cond, No *corpo);
No* noSeSenao(No *cond, No *corpoSe, No *corpoSenao);
No* noEnquanto(No *cond, No *corpo);
No* noFacaEnquanto(No *corpo, No *cond);
No* noPara(const std::string &var, No *inicio, No *fim, No *passo, No *corpo);
No* noQuebra();
No* noContinua();
No* noAtribuicao(const std::string &nome, No *expr);
No* noBinario(const std::string &op, No *esq, No *dir);
No* noUnario(const std::string &op, No *operando);
No* noIdent(const std::string &nome);
No* noLiteral(const Valor &v);

#endif