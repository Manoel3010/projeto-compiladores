# Roda o compilador dentro do Docker, sem instalar nada no Windows.
#
#   .\teste.ps1                                 bateria completa
#   .\teste.ps1 flex     testes/ok_pipeline.ling  analisador lexico (tokens)
#   .\teste.ps1 bison    testes/ok_for.ling       analisador sintatico + execucao
#   .\teste.ps1 gramatica                         conflitos e relatorio do bison
#   .\teste.ps1 compilar                          so compila
#   .\teste.ps1 shell                             abre um terminal dentro do container

param(
    [string]$acao = "tudo",
    [string]$arquivo = ""
)

$ErrorActionPreference = "Stop"
$projeto = $PSScriptRoot
$imagem  = "ling-build"

# --- garante que a imagem existe ---------------------------------------
docker image inspect $imagem 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Imagem '$imagem' nao encontrada. Construindo (so' na primeira vez)..." -ForegroundColor Yellow
    $tmp = Join-Path $env:TEMP "ling-build-ctx"
    New-Item -ItemType Directory -Force -Path $tmp | Out-Null
    @"
FROM debian:stable-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends g++ make flex bison \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /proj
"@ | Out-File -FilePath (Join-Path $tmp "Dockerfile") -Encoding utf8
    docker build -t $imagem $tmp
    if ($LASTEXITCODE -ne 0) { Write-Host "Falha ao construir a imagem." -ForegroundColor Red; exit 1 }
}

# Preparo comum: copia o projeto para /work, normaliza CRLF e compila.
# O projeto entra somente-leitura, entao sua pasta no Windows nao e' tocada.
$preparo = "cp -a /src/. /work/; find . -type f \( -name '*.sh' -o -name '*.ling' \) -exec sed -i 's/\r`$//' {} + 2>/dev/null; make"

switch ($acao.ToLower()) {

    "shell" {
        Write-Host "Abrindo o container. Rode primeiro:  cp -a /src/. /work/ && make" -ForegroundColor Cyan
        docker run --rm -it -v "${projeto}:/src:ro" -w /work $imagem bash
        exit $LASTEXITCODE
    }

    "compilar" { $cmd = "$preparo" }

    "tudo"     { $cmd = "$preparo >/dev/null 2>&1; sh testes/run.sh" }

    "flex" {
        if (-not $arquivo) { Write-Host "Uso: .\teste.ps1 flex <arquivo.ling>" -ForegroundColor Red; exit 1 }
        $cmd = "$preparo >/dev/null 2>&1; ./linguagem --tokens '$arquivo'"
    }

    "bison" {
        if (-not $arquivo) { Write-Host "Uso: .\teste.ps1 bison <arquivo.ling>" -ForegroundColor Red; exit 1 }
        $cmd = "$preparo >/dev/null 2>&1; ./linguagem '$arquivo'"
    }

    "gramatica" {
        $cmd = "$preparo >/dev/null 2>&1; echo '--- conflitos (silencio = nenhum) ---'; bison -d -v -o /tmp/p.tab.c projeto/src/parser.y && echo 'zero conflitos'; echo ''; echo '--- build/parser.output, primeiras 30 linhas ---'; head -30 build/parser.output"
    }

    default {
        Write-Host "Acao desconhecida: $acao" -ForegroundColor Red
        Write-Host "Use: tudo | flex | bison | gramatica | compilar | shell"
        exit 1
    }
}

docker run --rm -v "${projeto}:/src:ro" -w /work $imagem sh -c $cmd
exit $LASTEXITCODE
