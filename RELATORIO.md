# Relatório — Analisador Léxico e Sintático

## O que este programa faz

Ele lê um arquivo de código e responde uma única pergunta: **este código está
escrito corretamente?**

- Se estiver: imprime `Análise concluída com sucesso` e termina com código 0.
- Se não estiver: imprime a mensagem de erro **com o número da linha** e termina
  com código 1.

Isso se chama **reconhecedor**. Ele não executa o código, não calcula nada e não
traduz para outra linguagem — apenas verifica se a escrita obedece às regras.

**Escopo desta etapa:** só as duas primeiras fases de um compilador, a análise
léxica e a sintática. Não há árvore sintática (AST), tabela de símbolos, código
intermediário nem interpretador. Por isso os arquivos `ast.cpp`, `interp.cpp` e
`symtab.cpp` continuam vazios.

### As duas fases, em uma frase cada

| Fase | Ferramenta | Arquivo | O que faz |
| --- | --- | --- | --- |
| **Léxica** | Flex | `lexer.l` | Quebra o texto em palavras (*tokens*): vê `for`, `i`, `0`, `..` |
| **Sintática** | Bison | `parser.y` | Confere se essas palavras estão em ordem válida |

Um erro **léxico** é uma palavra que não existe na linguagem (ex.: o caractere
`@`). Um erro **sintático** é uma palavra válida no lugar errado (ex.: `;`
faltando).

---

## A linguagem

O diferencial é o laço `for` com intervalo (*range*):

```
for i in 0..10 step 2 { }
```

Exemplo completo de programa válido:

```
{
    int x;
    float media;

    x = 10;
    media = (x + 3) / 2;

    if (x > 5) {
        x = x - 1;
    } else {
        x = 0;
    }

    for i in 0..10 step 2 {
        x = x + i;
    }
}
```

Todo programa é um bloco `{ }`, com as declarações primeiro e os comandos depois.

**Palavras-chave (15):** `if else while do for in step break continue true false
int float bool char`

**Operadores:** `&& || ! == != <= >= < > = + - * / %` e o operador de intervalo `..`

**Comentários:** `// até o fim da linha` e `/* de várias linhas */`

---

## Como usar

> Antes de qualquer comando, veja o **Passo 0** do roteiro de teste: é preciso
> estar no terminal certo e na pasta certa. Os comandos abaixo assumem isso.

### Compilar

```sh
make
```

### Três modos de uso

```sh
./linguagem.exe arquivo.ling             # analisa um arquivo
./linguagem.exe                          # analisa o que for digitado (stdin)
./linguagem.exe --tokens arquivo.ling    # só lista os tokens, sem analisar sintaxe
```

O modo `--tokens` existe para testar o Flex sozinho: ele chama `yylex()` num laço
e imprime cada token encontrado, **sem chamar o `yyparse()`**. Serve para ver
exatamente como o texto foi quebrado em palavras.

### Rodar todos os testes

```sh
./testes/run.sh
```

---

## O que foi feito

### Ponto de partida

| Item | Situação inicial |
| --- | --- |
| `lexer.l` e `parser.y` | calculadora de brinquedo (só `NUM`, `ID` e `+`) |
| `main.cpp` | só chamava `yyparse()` |
| `ast.cpp`, `interp.cpp`, `symtab.cpp` | vazios |
| Makefile | não existia |
| `lex.yy.c`, `parser.tab.c`, `parser.tab.h`, `linguagem.exe` | versionados na raiz |

### Arquivos reescritos

**`projeto/src/lexer.l`** (101 linhas) — o analisador léxico.

- Reconhece as 15 palavras-chave, identificadores, números inteiros (`NUM`),
  números reais (`REAL`), o operador `..` (`RANGE`), todos os operadores e
  delimitadores.
- Ignora espaços, tabulações, quebras de linha e comentários.
- Conta as linhas automaticamente (`%option yylineno`), inclusive as que estão
  dentro de comentários `/* */`.
- Avisa se um comentário `/*` nunca for fechado.
- **Todos os `printf` de depuração foram removidos** — o lexer agora só devolve
  tokens, em silêncio.
- Caractere desconhecido gera erro com o número da linha, e a análise continua
  (para reportar vários erros de uma vez).

**`projeto/src/parser.y`** (116 linhas) — o analisador sintático. Contém a
gramática completa e as regras de precedência. Sem `%union`, sem `%type` e sem
ações semânticas, conforme o escopo.

**`projeto/src/main.cpp`** (106 linhas) — lê o arquivo ou o stdin, decide entre o
modo normal e o `--tokens`, e define o código de saída.

### Arquivos criados

| Arquivo | Para que serve |
| --- | --- |
| `Makefile` | compila tudo com um comando |
| `.gitignore` | impede que arquivos gerados voltem para o repositório |
| `testes/run.sh` | roda a bateria de testes e resume o resultado |
| `testes/ok_exemplo.ling` | declarações, if/else, while, do-while, precedência |
| `testes/ok_for.ling` | `for` simples, com `step`, aninhado, `break` e `continue` |
| `testes/ok_range.ling` | `0..10` e `0.5` no mesmo arquivo |
| `testes/erro_ponto_virgula.ling` | erro proposital: `;` faltando |
| `RELATORIO.md` | este arquivo |

### Sobre o Makefile

- Os arquivos gerados vão **todos** para `build/`. A raiz fica limpa, só com o
  binário.
- `FLEX ?= flex` e `BISON ?= bison` — quem usa as versões `win_flex` /
  `win_bison` pode trocar sem editar o arquivo:
  `make FLEX=win_flex BISON=win_bison`
- O binário é `linguagem.exe` no Windows e `linguagem` no Linux.
- `make clean` apaga tudo que foi gerado.

### Sobre o `run.sh`

Ele usa uma convenção simples de nomes:

- `testes/ok_*.ling` → **tem que ser aceito**
- `testes/erro_*.ling` → **tem que ser rejeitado**

Para cada arquivo imprime `PASS` ou `FALHA`, e no fim um resumo. O código de
saída é o número de falhas — `0` significa que passou tudo. Para adicionar um
teste novo basta criar o arquivo com o prefixo certo; o script o encontra
sozinho.

### Limpeza do repositório

Os quatro arquivos gerados que estavam versionados foram removidos do controle de
versão (mas continuam no disco):

```sh
git rm --cached lex.yy.c parser.tab.c parser.tab.h linguagem.exe
```

**Por quê:** arquivo gerado não é código-fonte. Deixá-lo versionado causa
conflito de merge toda vez que alguém mexe no `.l` ou no `.y`.

**Consequência:** quem clonar o repositório agora precisa ter o Flex e o Bison
instalados para rodar o `make`. Antes não precisava, porque o `lex.yy.c` e o
`parser.tab.c` já vinham prontos.

---

## A única mudança na gramática, e por quê

A gramática pedida tinha esta regra para o `for`:

```
FOR ID IN expr RANGE expr stmt
```

Ou seja, o corpo do `for` podia ser **qualquer comando** (`stmt`). Isso gerava
**10 conflitos** no Bison.

### O que é um conflito

É quando o analisador chega num ponto e a gramática permite **duas
interpretações diferentes** para a mesma entrada. Ele não tem como escolher.

Aqui o problema era o sinal de menos. Na entrada:

```
for i in 0..10 -x
```

O `-` pode ser duas coisas:

1. **Subtração**: o limite do intervalo é `10 - x`, e o corpo do `for` ficou
   faltando.
2. **Menos unário**: o intervalo é `0..10` e o corpo é o comando `-x`.

Ambas seguem a gramática. Isso é ambiguidade de verdade, não um detalhe de
configuração.

### Por que não deu para resolver com precedência

As declarações `%left`, `%right` e `%nonassoc` só funcionam para conflitos do
tipo *shift/reduce*. Este era do tipo *reduce/reduce*, e nesses o Bison **ignora
a precedência** e escolhe pela ordem em que as regras foram escritas. Não existe
ajuste de precedência que resolva.

**Efeito prático:** `for x in 0..10 -x + 1;` era recusado, mesmo sendo válido.

### A correção

O corpo do `for` passou a exigir chaves:

```
FOR ID IN expr RANGE expr block
FOR ID IN expr RANGE expr STEP expr block
```

Como um bloco sempre começa com `{`, não há mais dúvida: se vier `-`, é
subtração. Os 10 conflitos foram para zero.

A sintaxe continua igual à do exemplo da linguagem (`for i in 0..10 step 2 { }`).
O único custo é que `for` sem chaves não é mais aceito — restrição que o `if` e o
`while` não têm.

### O resto da gramática

Toda a ambiguidade das expressões foi resolvida **só com precedência**, sem
quebrar a regra em camadas `rel`/`ari`/`term`:

```
%right '='
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right '!' UMINUS
```

De cima para baixo, da menor para a maior prioridade. É isso que faz
`x + y * 2 > 15 && !false` ser lido como `((x + (y*2)) > 15) && (!false)`.

O caso clássico do **`else` solto** (quando há dois `if` e um só `else`, a qual
deles ele pertence?) foi resolvido com `%nonassoc LOWER_THAN_ELSE` e
`%nonassoc ELSE`: o `else` liga sempre ao `if` mais próximo.

**Resultado final: 0 conflitos shift/reduce e 0 conflitos reduce/reduce.**

---

## Roteiro de teste

### Passo 0 — Terminal e pasta

**Terminal: use o Git Bash ou o MSYS2 MINGW64.** O projeto usa `make` e um script
`.sh`, que são ferramentas Unix — elas não funcionam no PowerShell nem no Prompt
de Comando. Para conferir: o prompt do Git Bash começa com `$`; o do PowerShell
começa com `PS C:\...>`.

**Pasta: rode tudo de dentro de `projeto-compiladores`**, a pasta que contém o
`Makefile`. Cuidado, o repositório costuma ficar dentro de outra pasta:

```
projeto/                        ← NÃO é aqui
└── projeto-compiladores/       ← é AQUI que os comandos rodam
    ├── Makefile
    ├── projeto/src/            ← o código-fonte mora aqui
    └── testes/
```

Confirme com `ls Makefile testes/run.sh` — tem que responder os dois nomes. Se
der `No such file or directory`, você está um nível acima: `cd projeto-compiladores`.

#### Se der erro

| Mensagem | Causa |
| --- | --- |
| `'sh' não é reconhecido...` | está no PowerShell — abra o Git Bash |
| `'make' não é reconhecido...` | terminal errado, ou Flex/Bison não instalados (Passo 1) |
| `bash: ./testes/run.sh: No such file or directory` | pasta errada; ou o script está com quebras de linha do Windows — corrija com `sed -i 's/\r$//' testes/run.sh` |

---

### Lista de comandos de teste

Resumo de tudo o que dá para rodar. Cada um está detalhado nos passos seguintes.

| Comando | O que faz |
| --- | --- |
| `make` | Compila o projeto e gera o `linguagem.exe` |
| `make clean` | Apaga o `build/` e o binário, para recompilar do zero |
| `./testes/run.sh` | **Roda todos os testes** e resume quantos passaram |
| `./linguagem.exe arquivo.ling` | Analisa um arquivo: diz se está certo ou aponta o erro |
| `./linguagem.exe` | O mesmo, lendo o que for digitado no teclado |
| `./linguagem.exe --tokens arquivo.ling` | Lista os tokens do arquivo, **sem** analisar a sintaxe — testa só o Flex |
| `grep -ni conflit build/parser.output` | **Confere que a gramática não tem conflitos** — não imprimir nada é o resultado certo |
| `grep -ci -e "^state " -e "^estado " build/parser.output` | Conta os estados da tabela do analisador (93) |
| `grep -ni -A 12 -e "^state 6$" -e "^estado 6$" build/parser.output` | Mostra um estado específico da tabela, para investigar um conflito |

---

### Passo 1 — Instalar as ferramentas

São necessários **flex**, **bison**, **g++** e **make**.

**MSYS2** (recomendado, versões atuais):
```sh
pacman -S flex bison make mingw-w64-x86_64-gcc
```
Depois use o terminal *MINGW64*.

**MinGW.org** (mais enxuto, versões antigas):
```sh
mingw-get install msys-flex msys-bison
```
Acrescente `C:\MinGW\bin` e `C:\MinGW\msys\1.0\bin` ao `PATH`.

**Linux:**
```sh
sudo apt install flex bison g++ make
```

Confirme que tudo respondeu:
```sh
flex --version && bison --version && g++ --version && make --version
```

---

### Passo 2 — Compilar

No Git Bash ou no MSYS2, dentro da pasta `projeto-compiladores`:

```sh
make clean
make
```

Saída esperada:

```
mkdir build
bison -d -v -o build/parser.tab.c projeto/src/parser.y
g++ -Wall -Wno-unused-function -Ibuild -c -o build/parser.tab.o build/parser.tab.c
flex -o build/lex.yy.c projeto/src/lexer.l
g++ -Wall -Wno-unused-function -Ibuild -c -o build/lex.yy.o build/lex.yy.c
g++ -Wall -Wno-unused-function -Ibuild -c -o build/main.o projeto/src/main.cpp
g++ -Wall -Wno-unused-function -o linguagem.exe build/parser.tab.o build/lex.yy.o build/main.o
```

**Repare que não há nenhuma linha a mais.** Se houvesse conflitos na gramática, o
Bison reclamaria logo depois da linha dele. Silêncio ali já é o primeiro sinal de
que está tudo certo.

---

### Passo 3 — Testar o **Bison** (análise sintática)

A opção `-v` faz o Bison gerar um relatório em `build/parser.output`, com a
tabela completa do analisador. É ali que se confirma que **não há conflitos**.

```sh
grep -ni conflit build/parser.output
```

**Não imprimir nada = 0 conflitos.** É o resultado esperado.

Se houvesse algum, apareceria na primeira linha do arquivo, assim:

```
State 93 conflicts: 10 reduce/reduce
```

> **Cuidado:** o Bison 3.x traduz esse relatório. Num Windows em português ele
> escreve `Estado` e `conflitos`, não `State` e `conflicts`. Por isso o comando
> acima procura `conflit`, que é o pedaço comum às duas línguas — procurar por
> `conflict` daria "nenhum conflito" só porque a palavra está em português.
> Para forçar tudo em inglês: `LC_ALL=C bison -d -v -o build/parser.tab.c projeto/src/parser.y`

**Ver o tamanho da tabela** (a gramática atual gera 93 estados, de 0 a 92):

```sh
grep -ci -e "^state " -e "^estado " build/parser.output
```

**Examinar um estado**, caso apareça algum conflito depois de uma alteração
(troque o `6` pelo número que o Bison citar):

```sh
grep -ni -A 12 -e "^state 6$" -e "^estado 6$" build/parser.output
```

```
185:state 6
187-    6 type: INT .
189-    $default  reduce using rule 6 (type)
```

---

### Passo 4 — Testar o **Flex** (análise léxica)

O modo `--tokens` isola o Flex: ele lista os tokens sem envolver o analisador
sintático.

```sh
./linguagem.exe --tokens testes/ok_range.ling
```

```
LINHA  TOKEN      LEXEMA
1      {          {
2      FLOAT      float
2      ID         f
2      ;          ;
...
9      FOR        for
9      ID         i
9      IN         in
9      NUM        0
9      RANGE      ..
9      NUM        10
```

Três colunas: a **linha** onde o token está, o **nome do token** e o **lexema**
(o texto exato que apareceu no arquivo). Tokens de um caractere aparecem pelo
próprio símbolo (`{`, `=`, `;`).

#### 4a — O teste mais importante: `0..10`

Este é o ponto delicado do lexer. A regra de número real é `[0-9]+\.[0-9]+`, e
ela vem antes da regra de número inteiro. O risco é o Flex ler `0.` como início
de um real e se perder no `0..10`.

```sh
printf 'for i in 0..10 step 2\n0.5\n1.25..3\n' | ./linguagem.exe --tokens
```

Resultado correto:

```
0..10    →  NUM 0  |  RANGE ..  |  NUM 10
0.5      →  REAL 0.5
1.25..3  →  REAL 1.25  |  RANGE ..  |  NUM 3
```

**Por que funciona:** ao ler `0..10`, o Flex tenta casar a regra de real, chega
em `0.` e vê que o próximo caractere é outro ponto, não um dígito. A regra falha,
então ele **volta atrás** (*backtrack*) e usa a regra de inteiro, casando só o
`0`. Depois casa `..` e por fim `10`.

A terceira linha (`1.25..3`) é o caso mais difícil: um real **seguido** de um
intervalo. Se algo estivesse errado, apareceria um `REAL` indevido ou um erro de
caractere desconhecido.

#### 4b — Erro léxico

```sh
printf '{\n  int a;\n  a = 1 @ 2;\n}\n' | ./linguagem.exe
```

```
Erro lexico na linha 3: caractere desconhecido '@'
```

#### 4c — Contagem de linhas dentro de comentários

Um comentário `/* */` de várias linhas precisa ser contado, senão todos os
números de linha depois dele saem errados.

```sh
printf '{\n/* c2\n   c3\n   c4 */\n  int a\n}\n' | ./linguagem.exe --tokens
```

```
LINHA  TOKEN      LEXEMA
1      {          {
5      INT        int
5      ID         a
6      }          }
```

O `int` está na linha 5 do arquivo. Ele aparecer como **5**, e não como 2, prova
que as três linhas do comentário foram contadas.

#### 4d — Comentário nunca fechado

```sh
printf '{\n  int a;\n  /* nunca fecha\n' | ./linguagem.exe
```

```
Erro lexico na linha 4: comentario /* nao fechado
Erro sintatico na linha 4: syntax error (proximo a '')
```

Duas mensagens: o erro léxico e, em seguida, o erro sintático causado pelo
arquivo ter acabado no meio.

---

### Passo 5 — Rodar a bateria completa

```sh
./testes/run.sh
```

```
Testes que devem ser ACEITOS
---------------------------------------------------------------
PASS   testes/ok_exemplo.ling     aceito
PASS   testes/ok_for.ling         aceito
PASS   testes/ok_range.ling       aceito

Testes que devem ser REJEITADOS
---------------------------------------------------------------
PASS   testes/erro_ponto_virgula.ling rejeitado: Erro sintatico na linha 7: syntax error (proximo a 'x')

===============================================================
4 testes: 4 passaram, 0 falharam
```

Conferir o código de saída (deve ser `0`):

```sh
./testes/run.sh; echo "exit code: $?"
```

**Confirmar que o teste não está passando à toa.** Vale plantar dois arquivos
errados de propósito — um inválido com nome `ok_` e um válido com nome `erro_` —
e verificar que o script acusa os dois:

```sh
printf '{ int x; x = ; }\n'  > testes/ok_SANIDADE.ling
printf '{ int x; x = 1; }\n' > testes/erro_SANIDADE.ling
./testes/run.sh; echo "exit code: $?"      # deve acusar 2 falhas e sair com 2
rm testes/ok_SANIDADE.ling testes/erro_SANIDADE.ling
```

---

## Uma observação sobre números de linha

No exemplo do passo 4c, o `;` está faltando depois de `int a` (linha 5), mas o
erro é reportado na **linha 6**:

```
Erro sintatico na linha 6: syntax error (proximo a '}')
```

Isso está correto. Um analisador sintático só percebe que faltou o `;` quando lê
o token **seguinte** — o `}` da linha 6. A linha reportada é sempre onde o erro
foi *percebido*, que às vezes é depois de onde ele foi *cometido*.

---

## Validação

O projeto foi compilado e testado em duas instalações completas e diferentes:

| | MinGW.org | MSYS2 |
| --- | --- | --- |
| flex | 2.5.35 | 2.6.4 |
| bison | 2.4.2 | 3.8.2 |
| g++ | 6.3.0 (C++14) | 16.1.0 (C++20) |
| Conflitos | 0 | 0 |
| Avisos com `-Wall` | nenhum | nenhum |
| `run.sh` | 4/4 | 4/4 |
| `make clean` + recompilar | ok | ok |

Também foi testada a compilação via `mingw32-make` direto do PowerShell.

Durante esses testes apareceu um problema: a variável `OS` do Windows **não é
repassada** pelo terminal do MSYS2, e o Makefile dependia dela para decidir entre
`linguagem` e `linguagem.exe`. Foi corrigido consultando o `uname` como segunda
tentativa.

**Não testado:** Linux de verdade. A lógica existe e funciona, mas nenhuma
máquina Linux compilou o projeto.

---

## Estrutura final

```
projeto-compiladores/
├── Makefile                       criado
├── RELATORIO.md                   criado
├── .gitignore                     criado
├── projeto/src/
│   ├── lexer.l                    reescrito   (101 linhas)
│   ├── parser.y                   reescrito   (116 linhas)
│   ├── main.cpp                   reescrito   (106 linhas)
│   ├── ast.cpp                    vazio, fora do escopo
│   ├── interp.cpp                 vazio, fora do escopo
│   └── symtab.cpp                 vazio, fora do escopo
├── testes/
│   ├── run.sh                     criado
│   ├── ok_exemplo.ling            criado
│   ├── ok_for.ling                criado
│   ├── ok_range.ling              criado
│   └── erro_ponto_virgula.ling    criado
└── build/                         gerado, fora do versionamento
```
