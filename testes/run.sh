#!/bin/sh
# Roda a bateria de testes:
#   testes/ok_*.ling    -> devem ser ACEITOS   (codigo de saida 0)
#   testes/erro_*.ling  -> devem ser REJEITADOS (codigo de saida diferente de 0)
#
# O codigo de saida do script e o numero de falhas (0 = tudo passou).

# Roda sempre a partir da raiz do projeto, independente de onde foi chamado.
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

    total=$((total + 1))
    saida=$("$BIN" "$arquivo" 2>&1)
    codigo=$?
    primeira_linha=$(echo "$saida" | head -1)

    if [ "$esperado" = ok ]; then
        if [ "$codigo" -eq 0 ]; then
            printf 'PASS   %-26s aceito\n' "$arquivo"
        else
            printf 'FALHA  %-26s deveria ser aceito (saida %d)\n' "$arquivo" "$codigo"
            printf '         %s\n' "$primeira_linha"
            falhas=$((falhas + 1))
        fi
    else
        if [ "$codigo" -ne 0 ]; then
            printf 'PASS   %-26s rejeitado: %s\n' "$arquivo" "$primeira_linha"
        else
            printf 'FALHA  %-26s deveria ser rejeitado, mas foi aceito\n' "$arquivo"
            falhas=$((falhas + 1))
        fi
    fi
}

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
