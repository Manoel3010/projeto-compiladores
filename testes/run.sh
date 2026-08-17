#!/bin/sh
# Roda a bateria de testes:
#   testes/ok_*.ling    -> devem ser ACEITOS   (codigo de saida 0)
#   testes/erro_*.ling  -> devem ser REJEITADOS (codigo de saida diferente de 0)
#
# Verificacoes opcionais, ativadas apenas se o arquivo existir ao lado do .ling:
#   testes/ok_X.saida   -> a saida padrao de "./linguagem ok_X.ling" tem que ser
#                          IDENTICA a esse arquivo. E' o que impede regressao
#                          silenciosa: sem isso um teste passa desde que nao
#                          quebre, mesmo calculando o valor errado.
#   testes/ok_X.tokens  -> a saida de "./linguagem --tokens ok_X.ling" tem que
#                          ser identica (teste unitario do analisador lexico).
#
# Para (re)gerar os esperados depois de conferir a saida na mao:
#   ./linguagem testes/ok_X.ling          > testes/ok_X.saida
#   ./linguagem --tokens testes/ok_X.ling > testes/ok_X.tokens
#
# O codigo de saida do script e o numero de falhas (0 = tudo passou).

cd "$(dirname "$0")/.." || exit 1

if [ -x ./linguagem.exe ]; then
    BIN=./linguagem.exe
elif [ -x ./linguagem ]; then
    BIN=./linguagem
else
    echo "Binario nao encontrado. Rode 'make' antes."
    exit 1
fi

total=0
falhas=0

verifica() {
    arquivo=$1
    esperado=$2
    base=${arquivo%.ling}

    total=$((total + 1))
    saida=$("$BIN" "$arquivo" 2>&1)
    codigo=$?
    primeira_linha=$(echo "$saida" | head -1)

    if [ "$esperado" = ok ]; then
        if [ "$codigo" -ne 0 ]; then
            printf 'FALHA  %-28s deveria ser aceito (saida %d)\n' "$arquivo" "$codigo"
            printf '         %s\n' "$primeira_linha"
            falhas=$((falhas + 1))
            return
        fi

        if [ -f "$base.saida" ]; then
            if "$BIN" "$arquivo" 2>/dev/null | diff "$base.saida" - > /tmp/dif.$$ 2>&1; then
                printf 'PASS   %-28s aceito + saida conferida\n' "$arquivo"
            else
                printf 'FALHA  %-28s saida diferente da esperada\n' "$arquivo"
                head -10 /tmp/dif.$$ | sed 's/^/         /'
                falhas=$((falhas + 1))
            fi
            rm -f /tmp/dif.$$
        else
            printf 'PASS   %-28s aceito (sem .saida)\n' "$arquivo"
        fi
    else
        if [ "$codigo" -ne 0 ]; then
            printf 'PASS   %-28s rejeitado: %s\n' "$arquivo" "$primeira_linha"
        else
            printf 'FALHA  %-28s deveria ser rejeitado, mas foi aceito\n' "$arquivo"
            falhas=$((falhas + 1))
        fi
    fi
}

verifica_tokens() {
    esperado=$1
    base=${esperado%.tokens}
    arquivo="$base.ling"

    total=$((total + 1))

    if [ ! -f "$arquivo" ]; then
        printf 'FALHA  %-28s .tokens sem o .ling correspondente\n' "$esperado"
        falhas=$((falhas + 1))
        return
    fi

    if "$BIN" --tokens "$arquivo" 2>/dev/null | diff "$esperado" - > /tmp/dtok.$$ 2>&1; then
        printf 'PASS   %-28s tokens conferidos\n' "$arquivo"
    else
        printf 'FALHA  %-28s lista de tokens diferente da esperada\n' "$arquivo"
        head -10 /tmp/dtok.$$ | sed 's/^/         /'
        falhas=$((falhas + 1))
    fi
    rm -f /tmp/dtok.$$
}

echo "Teste unitario do analisador lexico (lista de tokens)"
echo "---------------------------------------------------------------"
achou=0
for esperado in testes/*.tokens; do
    [ -e "$esperado" ] || continue
    achou=1
    verifica_tokens "$esperado"
done
[ "$achou" -eq 0 ] && echo "(nenhum arquivo .tokens encontrado)"

echo
echo "Testes que devem ser ACEITOS"
echo "---------------------------------------------------------------"
for arquivo in testes/ok_*.ling; do
    [ -e "$arquivo" ] || continue
    verifica "$arquivo" ok
done

echo
echo "Testes que devem ser REJEITADOS"
echo "---------------------------------------------------------------"
for arquivo in testes/erro_*.ling; do
    [ -e "$arquivo" ] || continue
    verifica "$arquivo" erro
done

echo
echo "==============================================================="
printf '%d testes: %d passaram, %d falharam\n' \
    "$total" "$((total - falhas))" "$falhas"

exit $falhas
