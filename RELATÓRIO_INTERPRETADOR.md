# Relatório — Interpretador

## O que mudou desde o relatório anterior

O `RELATORIO.md` original descrevia um **reconhecedor**: um programa que só
respondia "este código está escrito corretamente?" — sem calcular nada, sem
executar nada.

Esta etapa transforma o projeto num **interpretador de verdade**. Agora, além
de validar léxica e sintaticamente, o programa:

- constrói uma **árvore de sintaxe abstrata (AST)** a partir do código-fonte;
- **executa** essa árvore: calcula expressões, roda laços, avalia condições,
  atribui variáveis;
- ao final, imprime o **valor de todas as variáveis** que sobraram no escopo
  global.

Os arquivos que antes estavam vazios de propósito (`ast.cpp`, `interp.cpp`,
`symtab.cpp`) agora têm conteúdo — essa era exatamente a lacuna que o
`RELATORIO.md` original apontava como "fora do escopo desta etapa".

### As fases do compilador, atualizadas

| Fase | Ferramenta/Arquivo | Situação |
| --- | --- | --- |
| Léxica | `lexer.l` | já existia; ganhou a tarefa de **guardar o valor** de cada token (não só o tipo) |
| Sintática | `parser.y` | já existia; ganhou **ações semânticas** que constroem a AST em vez de só validar |
| **Construção da AST** | `ast.h` / `ast.cpp` | **novo** |
| **Tabela de símbolos** | `symtab.h` / `symtab.cpp` | **novo** |
| **Execução** | `interp.h` / `interp.cpp` | **novo** |
| Representação de valores | `value.h` / `value.cpp` | **novo** — não fazia parte do plano original, foi necessário pra dar suporte às outras quatro |

---

## Decisões de projeto

Duas decisões foram tomadas antes de implementar, porque mudam bastante a
arquitetura do interpretador:

### 1. Como o resultado da execução é mostrado

A linguagem não tem nenhum comando de saída (não existe `print`). Em vez de
adicionar um comando novo à gramática, optou-se por **imprimir o valor final
de todas as variáveis do escopo global** ao terminar a execução com sucesso.

```
Execucao concluida. Valores finais das variaveis:
  soma       int    = 20
```

**Vantagem:** não mexe na gramática nem no lexer, o interpretador fica
desacoplado de qualquer sintaxe nova.
**Limitação:** só é possível ver o resultado final, não valores intermediários
durante a execução (ex.: dentro de um laço). Fica como possível melhoria
futura adicionar um `print(expr);`.

### 2. Escopo de variáveis

A gramática permite declarar variáveis dentro de qualquer bloco `{ }` —
inclusive dentro do corpo de um `if`, `while` ou `for`. Optou-se pela solução
tecnicamente correta: uma **pilha de escopos aninhados**, em vez de uma única
tabela global.

Cada bloco `{ }` executado empilha um escopo novo; ao sair do bloco, esse
escopo é descartado. Uma busca por variável percorre do escopo mais interno
para o mais externo. Isso significa que:

```
{
    int x;
    x = 1;
    if (true) {
        int x;     /* variavel DIFERENTE, sombra a de fora */
        x = 99;
    }
    /* aqui x volta a valer 1 */
}
```

é executado corretamente — a variável interna nunca vaza para fora, e some
assim que o bloco termina.

---

## Arquivos novos

### `value.h` / `value.cpp` — valores em tempo de execução

Define o tipo `Valor`: uma estrutura que representa qualquer dado que o
interpretador manipula (um `int`, `float`, `bool` ou `char`), junto com o
tipo desse dado.

```cpp
struct Valor {
    Tipo tipo;
    union { long i; double f; bool b; char c; };
};
```

Um `union` guarda um valor por vez, interpretado de formas diferentes
conforme o campo `tipo`.

Esse arquivo também concentra:

- **Conversão entre tipos** (`convertidoPara`) — necessária, por exemplo,
  quando um `int` é atribuído a uma variável `float`.
- **Os operadores da linguagem** (`operarBinario`, `operarUnario`) — soma,
  subtração, comparações, `&&`/`||`, etc. — já aplicando a regra de promoção
  de tipo: se um dos operandos for `float`, o resultado é calculado em
  ponto flutuante; senão, tudo é tratado como inteiro (mesma regra do C,
  simplificada).
- Detecção de **divisão e módulo por zero**, que interrompem a execução com
  um erro.

Nada disso existia antes: o reconhecedor original nunca precisava *guardar*
um valor, só confirmar que um número ou identificador tinha uma forma válida.

### `ast.h` / `ast.cpp` — a árvore de sintaxe abstrata

Define a estrutura que representa o programa depois de interpretado
sintaticamente. Em vez de uma classe C++ diferente para cada tipo de
comando/expressão, existe um único `struct No` genérico, com uma "etiqueta"
(`enum class NoTipo`) dizendo o que aquele nó representa:

```cpp
enum class NoTipo {
    BLOCO, DECL, EXPR_STMT, SE, SE_SENAO, ENQUANTO, FACA_ENQUANTO, PARA,
    QUEBRA, CONTINUA, ATRIBUICAO, BINARIO, UNARIO, IDENT, LITERAL, /* ... */
};

struct No {
    NoTipo tipo;
    std::vector<No*> filhos;
    std::string texto;    // nome de variavel, ou operador
    Valor       valor;    // usado em LITERAL
    Tipo        tipoDecl; // usado em DECL
};
```

Por exemplo, `if (x > 5) { y = 1; }` vira uma árvore:

```
No{SE}
├── No{BINARIO, ">"}
│   ├── No{IDENT, "x"}
│   └── No{LITERAL, 5}
└── No{BLOCO}
    └── No{ATRIBUICAO, "y"}
        └── No{LITERAL, 1}
```

As funções `noSe()`, `noBinario()`, `noBloco()` etc. são fábricas que montam
esses nós — usadas dentro das ações semânticas do `parser.y`.

### `symtab.h` / `symtab.cpp` — tabela de símbolos com pilha de escopos

Guarda as variáveis declaradas (nome, tipo, valor atual) usando uma **pilha**
de tabelas — uma por escopo ativo:

```cpp
class TabelaSimbolos {
public:
    void empilharEscopo();
    void desempilharEscopo();
    void declarar(const std::string &nome, Tipo tipo);
    Valor obter(const std::string &nome) const;
    void atribuir(const std::string &nome, const Valor &v);
private:
    std::vector<Escopo> pilha;
};
```

- `declarar()` cria a variável **só no escopo do topo da pilha**, e recusa
  redeclaração dentro do mesmo escopo.
- `obter()`/`atribuir()` procuram do escopo mais interno para o mais externo
  — é o que implementa a visibilidade de variáveis "de fora" dentro de um
  bloco aninhado.

### `interp.h` / `interp.cpp` — o executor da árvore (tree-walking interpreter)

O núcleo do interpretador. Duas funções recursivas que percorrem a AST:

- **`avaliar(No*)`** — para expressões, sempre devolve um `Valor`. Um nó
  `BINARIO` chama `avaliar()` recursivamente nos dois filhos e combina os
  resultados; um nó `IDENT` busca o valor na tabela de símbolos; um nó
  `LITERAL` devolve o valor guardado nele mesmo.
- **`executarComando(No*)`** — para comandos, não devolve valor: só *faz*
  algo (roda um `if`, repete um `while`, etc.).

Dois mecanismos merecem destaque:

**Sinal de controle de fluxo.** Para implementar `break`/`continue`, cada
comando devolve um `enum class Sinal { NORMAL, QUEBRA, CONTINUA }`. Um bloco
comum só repassa esse sinal para cima sem tratar; apenas os laços
(`while`/`do-while`/`for`) sabem o que fazer com `QUEBRA` (interrompem o
laço) e `CONTINUA` (pulam para a próxima repetição).

**Escopo automático via RAII.** A struct `GuardaEscopo` empilha um escopo no
construtor e desempilha no destrutor:

```cpp
struct GuardaEscopo {
    TabelaSimbolos &tabela;
    GuardaEscopo(TabelaSimbolos &t) : tabela(t) { tabela.empilharEscopo(); }
    ~GuardaEscopo() { tabela.desempilharEscopo(); }
};
```

Isso garante que o escopo é desempilhado mesmo se uma exceção for lançada no
meio da execução do bloco (ex.: divisão por zero) — sem precisar lembrar de
desempilhar manualmente em cada caminho de saída da função.

**`interpretar(No *raiz)`** é o único ponto de entrada chamado de fora: cria
a tabela de símbolos, executa o bloco raiz, e imprime o dump final das
variáveis do escopo global.

---

## Arquivos modificados

### `parser.y`

Antes, cada regra da gramática só confirmava que a sequência de tokens era
válida e descartava tudo. Agora cada regra **constrói um pedaço da AST** e
devolve para a regra de cima. Três adições tornaram isso possível:

```yacc
%union {
    long         num;
    double       real;
    std::string *str;
    No          *no;
    Tipo         tipoval;
}

%type <no>      program block decls decl stmts stmt expr
%type <tipoval> type
```

- **`%union`** define os tipos de dado que um token ou não-terminal pode
  carregar (um número, uma string, um ponteiro para nó da AST...).
- **`%type`** diz qual campo da union cada não-terminal usa — por exemplo,
  toda `expr` carrega um `No*`.
- **`%code requires { #include "ast.h" }`** garante que `No` e `Tipo` fiquem
  visíveis também no `parser.tab.h` gerado, porque o `lexer.l` precisa
  conhecer esses tipos para preencher o `yylval`.

Exemplo de regra, antes e depois:

```yacc
/* antes: so' confirma que "expr + expr" e' valido, e descarta */
expr : expr '+' expr ;

/* agora: monta um no' BINARIO e devolve pra cima */
expr : expr '+' expr { $$ = noBinario("+", $1, $3); } ;
```

A gramática em si (a estrutura das regras, a precedência de operadores, a
solução do `for` com bloco obrigatório, o `%nonassoc` do dangling else) **não
mudou** — só ganharam ações semânticas.

### `lexer.l`

Antes, ao reconhecer um número ou identificador, o lexer devolvia só o
**tipo** do token e esquecia o texto. Agora ele guarda o valor lido em
`yylval`, para o parser poder usar:

```c
/* antes */
{DIGITO}+  { return NUM; }

/* agora */
{DIGITO}+  { yylval.num = atol(yytext); return NUM; }
```

O mesmo vale para `ID` (guarda o nome como `std::string*`) e `REAL` (guarda
o valor como `double`, via `atof`).

### `main.cpp`

Uma mudança de uma linha, que liga o parser ao interpretador. Antes:

```cpp
} else {
    printf("Análise concluída com sucesso\n");
    codigo = 0;
}
```

Agora:

```cpp
} else {
    printf("Análise concluída com sucesso\n");
    codigo = interpretar(raizAst);
}
```

`raizAst` é uma variável global preenchida pelo `parser.y` (na regra
`program`) assim que a árvore inteira termina de ser construída.

### `Makefile`

Passou a compilar os quatro `.cpp` novos e linká-los no executável final:

```make
OBJS = $(BUILD)/parser.tab.o $(BUILD)/lex.yy.o $(BUILD)/main.o \
       $(BUILD)/value.o $(BUILD)/ast.o $(BUILD)/symtab.o $(BUILD)/interp.o
```

Também ganhou `-I$(SRC)` em `INCLUDES`, para os `#include "ast.h"` etc.
encontrarem os headers dentro de `projeto/src/`.

---

## Roteiro de teste

### Compilar

```sh
make clean
make
```

Sem nenhuma linha de erro — igual antes, o silêncio do Bison depois da linha
dele confirma que continuam **0 conflitos** na gramática (a gramática em si
não mudou).

### Casos testados manualmente

**`for` com `step`, acumulando soma:**

```sh
printf '{
    int soma;
    soma = 0;
    for i in 0..10 step 2 {
        soma = soma + i;
    }
}\n' | ./linguagem.exe
```

```
Análise concluída com sucesso
Execucao concluida. Valores finais das variaveis:
  soma       int    = 20
```

**`while` com `break` e `continue`:**

```sh
printf '{
    int i;
    int soma;
    i = 0; soma = 0;
    while (i < 10) {
        i = i + 1;
        if (i == 5) { continue; }
        if (i == 8) { break; }
        soma = soma + i;
    }
}\n' | ./linguagem.exe
```

```
Execucao concluida. Valores finais das variaveis:
  i          int    = 8
  soma       int    = 23
```

**Escopo aninhado (variável interna não vaza):**

```sh
printf '{
    int x;
    x = 1;
    if (true) {
        int x;
        x = 99;
    }
}\n' | ./linguagem.exe
```

```
Execucao concluida. Valores finais das variaveis:
  x          int    = 1
```

**Erros em tempo de execução** (divisão por zero, variável não declarada,
redeclaração no mesmo escopo) terminam a execução com código de saída `1` e
uma mensagem descritiva, por exemplo:

```
Erro em tempo de execucao: divisao por zero
```

### Arquivos de teste adicionados

| Arquivo | O que verifica |
| --- | --- |
| `testes/interp_for.ling` | `for` com `step`, acúmulo em variável |
| `testes/interp_while_break_continue.ling` | `while`, `break`, `continue` |
| `testes/interp_escopo_e_tipos.ling` | escopo aninhado, conversão int→float |

Esses arquivos passam pelo `testes/run.sh` como "aceitos" (validam a sintaxe);
a conferência do **valor calculado** é visual, olhando a saída do
`Execucao concluida...` no terminal.

---

## Uma observação sobre divisão inteira

Um detalhe que vale documentar porque pode parecer bug e não é: em
`media = (x + 3) / 2` com `x` inteiro, `(x + 3) / 2` é uma **divisão
inteira** (ex.: `13 / 2 = 6`), e só depois o resultado é convertido para
`float` ao ser guardado em `media`. É o mesmo comportamento do C. Se o
código precisar de divisão em ponto flutuante, um dos operandos precisa ser
`float` desde o início (ex.: `2.0` em vez de `2`).

---

## Limitações conhecidas / próximos passos

| Limitação | Detalhe |
| --- | --- |
| Sem literal de `char` | A gramática não tem token para `'a'`; uma variável `char` só recebe valor por atribuição de um `int` convertido |
| Sem saída durante a execução | Só o dump final das variáveis; não há como observar um valor no meio de um laço sem um comando `print` |
| Sem verificação de tipos em tempo de compilação | Erros de tipo (ex.: usar uma variável antes de declarar) só aparecem em **tempo de execução**, não durante a análise sintática |
| `%` com `float` | Não implementado (não faz sentido matemático direto); lança erro em tempo de execução se tentado |

---

## Estrutura final (arquivos deste projeto)

```
projeto-compiladores/
├── Makefile                              modificado
├── RELATORIO.md                          já existia (fase reconhecedor)
├── RELATORIO_INTERPRETADOR.md            este arquivo
├── projeto/src/
│   ├── lexer.l                           modificado
│   ├── parser.y                          modificado
│   ├── main.cpp                          modificado
│   ├── value.h / value.cpp               criados
│   ├── ast.h / ast.cpp                   criados
│   ├── symtab.h / symtab.cpp             criados
│   └── interp.h / interp.cpp             criados
├── testes/
│   ├── run.sh
│   ├── ok_exemplo.ling
│   ├── interp_for.ling                   criado
│   ├── interp_while_break_continue.ling  criado
│   └── interp_escopo_e_tipos.ling        criado
└── build/                                gerado, fora do versionamento
```