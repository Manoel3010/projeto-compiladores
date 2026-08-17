#include <stdio.h>
#include <string.h>
#include "parser.tab.h"
#include "ast.h"
#include "interp.h"

extern FILE *yyin;
extern int yyparse(void);
extern int yylex(void);
extern char *yytext;
extern int yylineno;
extern int erros_lexicos;
/* Preenchida por parser.y quando 'program' e' reduzido com sucesso. */
extern No *raizAst;

/* Nome legivel de um token, usado apenas pelo modo --tokens. */
static const char *nome_token(int t)
{
    static char simbolo[2];

    switch (t) {
    case IF:        return "IF";
    case ELSE:      return "ELSE";
    case WHILE:     return "WHILE";
    case DO:        return "DO";
    case FOR:       return "FOR";
    case IN:        return "IN";
    case STEP:      return "STEP";
    case BREAK:     return "BREAK";
    case CONTINUE:  return "CONTINUE";
    case INT:       return "INT";
    case FLOAT:     return "FLOAT";
    case BOOL:      return "BOOL";
    case CHAR:      return "CHAR";
    case TRUE:      return "TRUE";
    case FALSE:     return "FALSE";
    case ID:        return "ID";
    case NUM:       return "NUM";
    case REAL:      return "REAL";
    case RANGE:     return "RANGE";
    case PIPE:      return "PIPE";
    case OR:        return "OR";
    case AND:       return "AND";
    case EQ:        return "EQ";
    case NE:        return "NE";
    case LE:        return "LE";
    case GE:        return "GE";
    }

    /* Tokens de um caractere ('+', '{', ';' ...) sao devolvidos pelo proprio
       codigo ASCII, entao o nome do token e o caractere. */
    if (t > 0 && t < 256) {
        simbolo[0] = (char) t;
        simbolo[1] = '\0';
        return simbolo;
    }

    return "DESCONHECIDO";
}

/* Modo --tokens: chama yylex() em laco e imprime a lista de tokens.
   Nao chama yyparse(), ou seja, nao faz analise sintatica nenhuma. */
static int listar_tokens(void)
{
    printf("%-6s %-10s %s\n", "LINHA", "TOKEN", "LEXEMA");

    int t;
    while ((t = yylex()) != 0)
        printf("%-6d %-10s %s\n", yylineno, nome_token(t), yytext);

    return erros_lexicos > 0 ? 1 : 0;
}

int main(int argc, char **argv)
{
    /* Sem isso o stdout fica bufferizado e o stderr nao: ao redirecionar com
       '2>&1' as mensagens de erro apareciam ANTES do "Analise concluida com
       sucesso", invertendo a ordem real dos acontecimentos. */
    setvbuf(stdout, NULL, _IONBF, 0);

    int modo_tokens = 0;
    const char *caminho = NULL;
    FILE *arquivo = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0)
            modo_tokens = 1;
        else
            caminho = argv[i];
    }

    if (caminho != NULL) {
        arquivo = fopen(caminho, "r");
        if (arquivo == NULL) {
            fprintf(stderr, "Erro: nao foi possivel abrir o arquivo '%s'\n", caminho);
            return 1;
        }
        yyin = arquivo;
    }

    int codigo;

    if (modo_tokens) {
        codigo = listar_tokens();
    } else if (yyparse() != 0 || erros_lexicos > 0) {
        codigo = 1;
    } else {
        printf("Análise concluída com sucesso\n");
        codigo = interpretar(raizAst);
    }

    if (arquivo != NULL)
        fclose(arquivo);

    return codigo;
}
