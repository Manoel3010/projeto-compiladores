## Como testar

### 1. Recompilar

```sh
make clean
make
```

### 2. Léxico — conferir se `|>` vira um token só

```sh
echo 'x |> print;' > /tmp/teste.ling
./linguagem --tokens /tmp/teste.ling
```

Esperado:
```
LINHA  TOKEN      LEXEMA
1      ID         x
1      PIPE       |>
1      ID         print
1      ;          ;
```

### 3. Execução 

```sh
./linguagem testes/ok_pipeline.ling
```

Esperado:
```
Análise concluída com sucesso
10
13
10
10
Execucao concluida. Valores finais das variaveis:
  x          int    = 10
  y          int    = 3
```
(as quatro linhas de número vêm de `x |> print;`, `(x + y) |> print;` e
`x |> print |> print;`, nessa ordem — a última imprime `10` duas vezes.)

`testes/ok_pipeline.ling` — o prefixo `ok_` já diz que deve ser aceito
**e executar com sucesso**, código de saída `0`), porque a execução
funciona de ponta a ponta.

