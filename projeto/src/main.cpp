#include <stdio.h>

extern int yyparse();

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}

int main() {
    printf("Digite uma entrada:\n");

    int resultado = yyparse();

    printf("Resultado = %d\n", resultado);

    if (resultado == 0)
        printf("Entrada valida!\n");

    return 0;
}