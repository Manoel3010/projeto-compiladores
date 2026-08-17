# projeto-compiladores

Linguagem de programação própria com analisador léxico (Flex), analisador
sintático (Bison) e interpretador *tree-walking* em C++.

Os arquivos-fonte da linguagem usam a extensão `.ling`. Os diferenciais são o
laço `for` com intervalo e passo — `for i in 0..10 step 2 { }` — e o operador
de pipeline `|>`, onde `x |> f` equivale a `f(x)`.

---

## Começo rápido

Precisa apenas de **Docker** instalado e rodando. Não é necessário instalar
`flex`, `bison`, `g++` nem `make` no seu sistema.

Na raiz do projeto:

```powershell
# PowerShell
.\teste.ps1
```

```bash
# Git Bash, MSYS2, WSL ou Linux
./teste.sh
```

Saída esperada:

```
Teste unitario do analisador lexico (lista de tokens)
---------------------------------------------------------------
PASS   testes/ok_pipeline.ling      tokens conferidos
PASS   testes/ok_range.ling         tokens conferidos

Testes que devem ser ACEITOS
---------------------------------------------------------------
PASS   testes/ok_exemplo.ling       aceito + saida conferida
PASS   testes/ok_for.ling           aceito + saida conferida
PASS   testes/ok_pipeline.ling      aceito + saida conferida
PASS   testes/ok_prog_fibonacci.ling aceito + saida conferida
PASS   testes/ok_prog_mdc.ling      aceito + saida conferida
PASS   testes/ok_range.ling         aceito + saida conferida

Testes que devem ser REJEITADOS
---------------------------------------------------------------
PASS   testes/erro_ponto_virgula.ling rejeitado: Erro sintatico na linha 7: syntax error (proximo a 'x')

===============================================================
9 testes: 9 passaram, 0 falharam
```

`saida conferida` significa que o teste não apenas rodou sem quebrar: a saída
foi comparada byte a byte com o arquivo `.saida` guardado ao lado do `.ling`.

Na primeira execução o script constrói a imagem Docker sozinho, o que leva
cerca de um minuto. Nas seguintes, é imediato.

---

## Todos os comandos

| O que faz | PowerShell | Bash |
| --- | --- | --- |
| Roda a bateria completa | `.\teste.ps1` | `./teste.sh` |
| Analisador léxico (tokens) | `.\teste.ps1 flex <arquivo>` | `./teste.sh flex <arquivo>` |
| Analisador sintático + execução | `.\teste.ps1 bison <arquivo>` | `./teste.sh bison <arquivo>` |
| Conflitos e relatório da gramática | `.\teste.ps1 gramatica` | `./teste.sh gramatica` |
| Apenas compilar | `.\teste.ps1 compilar` | `./teste.sh compilar` |
| Terminal dentro do container | `.\teste.ps1 shell` | `./teste.sh shell` |

Os dois scripts têm exatamente as mesmas ações. Use o que corresponde ao seu
terminal.

---

## Testar tudo de uma vez

Compila do zero e roda os cinco casos da pasta `testes/`.

```powershell
.\teste.ps1
```

```bash
./teste.sh
```

O código de saída é o **número de falhas** — `0` significa tudo verde. É o que
permite usar o script direto numa pipeline de CI.

```powershell
.\teste.ps1; echo "falhas: $LASTEXITCODE"
```

```bash
./teste.sh; echo "falhas: $?"
```

---

## Rodar um teste de cada vez

A bateria roda os nove de uma vez, mas cada arquivo de `testes/` também roda
sozinho. Serve para investigar uma falha sem o ruído dos outros, ou para
mostrar um caso específico.

A ação `bison` analisa **e executa** o arquivo; a ação `flex` só lista os
tokens, sem chamar o parser.

### Executar (análise sintática + interpretador)

| Arquivo | Comando | Resultado esperado |
| --- | --- | --- |
| `ok_exemplo.ling` | `.\teste.ps1 bison testes/ok_exemplo.ling` | `x=0`, `y=11`, `media=1.5`, `ok=true` |
| `ok_for.ling` | `.\teste.ps1 bison testes/ok_for.ling` | `i=0`, `j=2`, `soma=1002` |
| `ok_range.ling` | `.\teste.ps1 bison testes/ok_range.ling` | `f=11.25`, `i=0` |
| `ok_pipeline.ling` | `.\teste.ps1 bison testes/ok_pipeline.ling` | imprime `10`, `13`, `10`, `10`; `x=10`, `y=3` |
| `ok_prog_fibonacci.ling` | `.\teste.ps1 bison testes/ok_prog_fibonacci.ling` | imprime `0 1 1 2 3 5 8 13 21 34`; `a=55`, `b=89`, `t=89` |
| `ok_prog_mdc.ling` | `.\teste.ps1 bison testes/ok_prog_mdc.ling` | imprime `6`; `a=6`, `b=0`, `t=6` |
| `erro_ponto_virgula.ling` | `.\teste.ps1 bison testes/erro_ponto_virgula.ling` | `Erro sintatico na linha 7`, código de saída `1` |

No Git Bash, WSL ou Linux é o mesmo comando trocando `.\teste.ps1` por
`./teste.sh`:

```bash
./teste.sh bison testes/ok_prog_mdc.ling
```

### Listar os tokens (análise léxica)

Qualquer um dos arquivos acima aceita a ação `flex`. Os dois que interessam:

| Arquivo | Comando | Para que serve |
| --- | --- | --- |
| `ok_range.ling` | `.\teste.ps1 flex testes/ok_range.ling` | confere que `0..10` vira `NUM 0` / `RANGE ..` / `NUM 10`, e que `0.5` continua um `REAL` só |
| `ok_pipeline.ling` | `.\teste.ps1 flex testes/ok_pipeline.ling` | confere que `\|>` é **um** token `PIPE`, não `\|` seguido de `>` |

Esses dois têm um `.tokens` guardado ao lado, e a bateria compara a lista
inteira automaticamente — não é conferência no olho.

### Exemplo completo

```powershell
.\teste.ps1 bison testes/ok_prog_mdc.ling
```

```
Análise concluída com sucesso
6
Execucao concluida. Valores finais das variaveis:
  a          int    = 6
  b          int    = 0
  t          int    = 6
```

O `6` no meio é o `a |> print;` do programa; o bloco final é o dump de
variáveis que o interpretador sempre imprime ao terminar.

---

## Testes individuais

### 1. Analisador léxico — Flex

Roda `yylex()` em laço e imprime linha, token e lexema. O parser não é
chamado.

```powershell
.\teste.ps1 flex testes/ok_pipeline.ling
```

```bash
./teste.sh flex testes/ok_pipeline.ling
```

```
LINHA  TOKEN      LEXEMA
11     ID         x
11     PIPE       |>
11     ID         print
11     ;          ;
```

É aqui que se confere se `|>` virou **um** token, e não `|` seguido de `>`.

### 2. Analisador sintático — Bison

Aciona o parser. Um arquivo válido imprime `Análise concluída com sucesso`;
um inválido imprime o erro com o número da linha e sai com código 1.

```powershell
.\teste.ps1 bison testes/erro_ponto_virgula.ling
```

```bash
./teste.sh bison testes/erro_ponto_virgula.ling
```

```
Erro sintatico na linha 7: syntax error (proximo a 'x')
```

> O binário tem apenas dois modos: `--tokens` (só o léxico) e a execução
> completa. Não existe um modo "analisar sem executar" — a linha
> `Análise concluída com sucesso` **é** o veredito do sintático, e tudo que
> aparece depois dela já vem do interpretador.

### 3. Interpretador

Mesmo comando do sintático, mas sobre um arquivo válido. Ao final, imprime os
valores das variáveis do escopo global.

```powershell
.\teste.ps1 bison testes/ok_for.ling
```

```bash
./teste.sh bison testes/ok_for.ling
```

```
Análise concluída com sucesso
Execucao concluida. Valores finais das variaveis:
  i          int    = 0
  j          int    = 2
  soma       int    = 1002
```

### 4. Gramática — conflitos do Bison

Não testa um programa, e sim a própria gramática. Silêncio do Bison significa
gramática sem ambiguidade.

```powershell
.\teste.ps1 gramatica
```

```bash
./teste.sh gramatica
```

```
--- conflitos (silencio = nenhum) ---
zero conflitos

--- build/parser.output, primeiras 30 linhas ---
Grammar

    0 $accept: program $end
    1 program: block
    ...
```

O relatório completo — estados, transições e eventuais conflitos — fica em
`build/parser.output`.

### 5. Apenas compilar

Útil para ver os avisos do compilador sem rodar teste nenhum.

```powershell
.\teste.ps1 compilar
```

```bash
./teste.sh compilar
```

---

## Trabalho exploratório: entrar no container

Cada chamada dos scripts recompila o projeto. Se for testar muita coisa em
sequência, compensa abrir um terminal dentro do container:

```powershell
.\teste.ps1 shell
```

```bash
./teste.sh shell
```

Uma vez lá dentro, prepare o ambiente:

```bash
cp -a /src/. /work/ && make
```

Daí em diante os comandos rodam direto, sem intermediário:

```bash
./linguagem --tokens testes/ok_pipeline.ling
./linguagem testes/ok_for.ling
sh testes/run.sh
```

Edições feitas no editor não aparecem sozinhas — repita `cp -a /src/. /work/
&& make` para ressincronizar.

---

## Sem Docker

Se preferir compilar nativamente, instale a toolchain:

```bash
# MSYS2 — depois use o terminal MINGW64
pacman -S flex bison make mingw-w64-x86_64-gcc

# Debian, Ubuntu, WSL
sudo apt install flex bison g++ make
```

Confirme:

```bash
flex --version && bison --version && g++ --version && make --version
```

E trabalhe direto na pasta do projeto:

```bash
make clean && make
bash testes/run.sh
./linguagem --tokens testes/ok_pipeline.ling
./linguagem testes/ok_for.ling
```

> **Atenção ao nome do binário.** No Windows o Makefile gera
> `linguagem.exe`; no Linux, WSL e no container, `linguagem`. Um executável
> compilado num ambiente não roda no outro — rode `make clean` ao alternar.

> **Git Bash não é MSYS2.** O prompt dos dois diz `MINGW64`, mas o Git Bash
> não tem compilador algum. Verifique com `command -v g++ flex bison make`:
> silêncio significa que não há toolchain nesse shell.

---

## Adicionar um teste

Não exige tocar em nenhum script — basta o nome do arquivo.

1. Crie o arquivo em `testes/` com o prefixo certo:
   - `ok_*.ling` — deve ser aceito **e executar até o fim** (código de saída `0`)
   - `erro_*.ling` — deve ser rejeitado (código diferente de `0`)
2. Rode a bateria. Ele já aparece na lista.
3. Opcional, mas recomendado: congele a saída esperada. Rode o teste sozinho,
   **confira o resultado na mão** e só então grave o `.saida`:

   ```bash
   ./linguagem testes/ok_meu_teste.ling > testes/ok_meu_teste.saida
   ```

   A partir daí a bateria compara a saída inteira, não só o código de saída.
   Para o léxico, o equivalente é o `.tokens`:

   ```bash
   ./linguagem --tokens testes/ok_meu_teste.ling > testes/ok_meu_teste.tokens
   ```

   Os dois arquivos são opcionais: sem eles o teste continua valendo, só que
   verificando menos. **Confira antes de congelar** — gravar uma saída errada
   deixa a bateria verde em cima de um bug.

O prefixo `ok_` promete duas coisas: que o arquivo compila e que ele executa
sem erro. Um erro em tempo de execução — divisão por zero, `step` igual a
zero — também derruba o teste, mesmo com a sintaxe impecável.

Lembre que a variável de um `for` vive em escopo próprio: `for j in 0..9`
cria um `j` novo que apenas **sombreia** o `j` declarado fora, sem alterá-lo.
Uma variável apenas declarada vale `0`.

---

## Quando algo quebra

**`bash: ./linguagem: No such file or directory`**
O binário não existe nessa pasta. Se você vinha usando os scripts, ele foi
gerado *dentro* do container e nunca esteve no seu sistema — use
`.\teste.ps1 bison <arquivo>` em vez de chamar o binário direto. Se compilou
nativamente no Windows, o nome é `linguagem.exe`.

**`Binário nao encontrado. Rode 'make' antes.`**
Mensagem do `testes/run.sh`, que procura `./linguagem.exe` e depois
`./linguagem`. Se sobrou um binário Linux chamado `linguagem` na pasta, ele
será escolhido no Windows e falhará de forma confusa. Apague-o e recompile.

**`Syntax error: "elif" unexpected`** ao rodar o `run.sh` no Linux
Não é erro do script, é quebra de linha. O `* text=auto` do `.gitattributes`
faz a árvore vir com CRLF no Windows, e o `dash` não tolera o `\r`. Os
scripts `teste.ps1` e `teste.sh` já normalizam isso automaticamente. Para
corrigir na origem, acrescente ao `.gitattributes`:

```
*.sh text eol=lf
```

**`não pode ser carregado porque a execução de scripts foi desabilitada`**
Política do PowerShell. Libere scripts locais para o seu usuário:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

**Mudei o `lexer.l` ou o `parser.y` e nada mudou**
Os scripts recompilam a cada chamada, então isso não deveria acontecer. Se
estiver trabalhando dentro do container ou nativamente, force o ciclo com
`make clean && make`.

---

## Estrutura

```
projeto/src/
  lexer.l       analisador léxico (Flex)
  parser.y      gramática e ações semânticas (Bison)
  ast.h/.cpp    nós da árvore de sintaxe abstrata
  value.h/.cpp  valores em tempo de execução e operadores
  symtab.h/.cpp tabela de símbolos com pilha de escopos
  interp.h/.cpp interpretador tree-walking
  main.cpp      linha de comando e modo --tokens

testes/
  ok_*.ling     programas que devem ser aceitos
  erro_*.ling   programas que devem ser rejeitados
  *.saida       saida esperada da execucao (opcional, comparada byte a byte)
  *.tokens      lista de tokens esperada (opcional, teste unitario do lexico)
  run.sh        roda a bateria e conta as falhas

teste.ps1       atalho para tudo, via Docker (PowerShell)
teste.sh        atalho para tudo, via Docker (bash)
Makefile        build; gera build/ e o binário
```

Os arquivos gerados (`build/`, `linguagem`, `linguagem.exe`) estão no
`.gitignore` e não devem ser commitados.
