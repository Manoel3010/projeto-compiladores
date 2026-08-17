#!/bin/sh
# Roda o compilador dentro do Docker, sem instalar nada no sistema.
#
#   ./teste.sh                                 bateria completa
#   ./teste.sh flex     testes/ok_pipeline.ling  analisador lexico (tokens)
#   ./teste.sh bison    testes/ok_for.ling       analisador sintatico + execucao
#   ./teste.sh gramatica                         conflitos e relatorio do bison
#   ./teste.sh compilar                          so compila
#   ./teste.sh shell                             abre um terminal dentro do container

set -e

# Roda sempre a partir da raiz do projeto, independente de onde foi chamado.
cd "$(dirname "$0")"
PROJETO=$(pwd)
IMAGEM=ling-build

# No Git Bash / MSYS2 o shell converte "/src" para um caminho do Windows e
# quebra a montagem. Esta variavel desliga essa conversao.
export MSYS_NO_PATHCONV=1

# Em Git Bash o pwd vem como /c/... ; o Docker precisa de C:/...
case "$PROJETO" in
    /[a-z]/*)
        letra=$(echo "$PROJETO" | cut -c2)
        resto=$(echo "$PROJETO" | cut -c3-)
        PROJETO=$(echo "$letra" | tr 'a-z' 'A-Z'):"$resto"
        ;;
esac

# --- garante que a imagem existe ---------------------------------------
if ! docker image inspect "$IMAGEM" >/dev/null 2>&1; then
    echo "Imagem '$IMAGEM' nao encontrada. Construindo (so' na primeira vez)..."
    ctx=$(mktemp -d)
    cat > "$ctx/Dockerfile" <<'EOF'
FROM debian:stable-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends g++ make flex bison \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /proj
EOF
    docker build -t "$IMAGEM" "$ctx"
    rm -rf "$ctx"
fi

# Preparo comum: copia o projeto para /work, normaliza CRLF e compila.
PREPARO="cp -a /src/. /work/; find . -type f \\( -name '*.sh' -o -name '*.ling' \\) -exec sed -i 's/\\r\$//' {} + 2>/dev/null; make"

ACAO=${1:-tudo}
ARQUIVO=${2:-}

case "$ACAO" in
    shell)
        echo "Abrindo o container. Rode primeiro:  cp -a /src/. /work/ && make"
        exec docker run --rm -it -v "$PROJETO:/src:ro" -w /work "$IMAGEM" bash
        ;;
    compilar)  CMD="$PREPARO" ;;
    tudo)      CMD="$PREPARO >/dev/null 2>&1; sh testes/run.sh" ;;
    flex)
        [ -n "$ARQUIVO" ] || { echo "Uso: ./teste.sh flex <arquivo.ling>"; exit 1; }
        CMD="$PREPARO >/dev/null 2>&1; ./linguagem --tokens '$ARQUIVO'"
        ;;
    bison)
        [ -n "$ARQUIVO" ] || { echo "Uso: ./teste.sh bison <arquivo.ling>"; exit 1; }
        CMD="$PREPARO >/dev/null 2>&1; ./linguagem '$ARQUIVO'"
        ;;
    gramatica)
        CMD="$PREPARO >/dev/null 2>&1; echo '--- conflitos (silencio = nenhum) ---'; bison -d -v -o /tmp/p.tab.c projeto/src/parser.y && echo 'zero conflitos'; echo ''; echo '--- build/parser.output, primeiras 30 linhas ---'; head -30 build/parser.output"
        ;;
    *)
        echo "Acao desconhecida: $ACAO"
        echo "Use: tudo | flex | bison | gramatica | compilar | shell"
        exit 1
        ;;
esac

docker run --rm -v "$PROJETO:/src:ro" -w /work "$IMAGEM" sh -c "$CMD"
