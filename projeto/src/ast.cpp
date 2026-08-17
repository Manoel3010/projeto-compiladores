#include "ast.h"

No* noLista()
{
    return new No(NoTipo::LISTA);
}

No* noBloco(No *decls, No *stmts)
{
    No *n = new No(NoTipo::BLOCO);
    /* 'decls' e 'stmts' sao nos LISTA temporarios (ver noLista); aqui a
       gente "achata" os dois numa lista so' de filhos do bloco, decls
       primeiro. O interpretador reconhece cada filho pelo proprio tipo
       (DECL ou nao), entao nao precisa guardar a fronteira entre os dois. */
    for (No *d : decls->filhos) n->filhos.push_back(d);
    for (No *s : stmts->filhos) n->filhos.push_back(s);
    delete decls;
    delete stmts;
    return n;
}

No* noDecl(Tipo tipo, const std::string &nome)
{
    No *n = new No(NoTipo::DECL);
    n->tipoDecl = tipo;
    n->texto = nome;
    return n;
}

No* noExprStmt(No *expr)
{
    No *n = new No(NoTipo::EXPR_STMT);
    n->filhos.push_back(expr);
    return n;
}

No* noSe(No *cond, No *corpo)
{
    No *n = new No(NoTipo::SE);
    n->filhos = { cond, corpo };
    return n;
}

No* noSeSenao(No *cond, No *corpoSe, No *corpoSenao)
{
    No *n = new No(NoTipo::SE_SENAO);
    n->filhos = { cond, corpoSe, corpoSenao };
    return n;
}

No* noEnquanto(No *cond, No *corpo)
{
    No *n = new No(NoTipo::ENQUANTO);
    n->filhos = { cond, corpo };
    return n;
}

No* noFacaEnquanto(No *corpo, No *cond)
{
    No *n = new No(NoTipo::FACA_ENQUANTO);
    n->filhos = { corpo, cond };
    return n;
}

No* noPara(const std::string &var, No *inicio, No *fim, No *passo, No *corpo)
{
    No *n = new No(NoTipo::PARA);
    n->texto = var;
    n->filhos = { inicio, fim, passo, corpo };
    return n;
}

No* noQuebra()   { return new No(NoTipo::QUEBRA); }
No* noContinua() { return new No(NoTipo::CONTINUA); }

No* noAtribuicao(const std::string &nome, No *expr)
{
    No *n = new No(NoTipo::ATRIBUICAO);
    n->texto = nome;
    n->filhos.push_back(expr);
    return n;
}

No* noBinario(const std::string &op, No *esq, No *dir)
{
    No *n = new No(NoTipo::BINARIO);
    n->texto = op;
    n->filhos = { esq, dir };
    return n;
}

No* noUnario(const std::string &op, No *operando)
{
    No *n = new No(NoTipo::UNARIO);
    n->texto = op;
    n->filhos.push_back(operando);
    return n;
}

No* noChamada(const std::string &nome, No *arg)
{
    No *n = new No(NoTipo::CHAMADA);
    n->texto = nome;
    n->filhos.push_back(arg);
    return n;
}

No* noIdent(const std::string &nome)
{
    No *n = new No(NoTipo::IDENT);
    n->texto = nome;
    return n;
}

No* noLiteral(const Valor &v)
{
    No *n = new No(NoTipo::LITERAL);
    n->valor = v;
    return n;
}