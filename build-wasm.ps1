# Compila o motor C++ para WebAssembly e gera os arquivos que o frontend importa.
# Uso (na raiz do projeto):
#   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
#   .\build-wasm.ps1
#
# O emsdk é procurado, nesta ordem: variável de ambiente EMSDK, pasta emsdk dentro do
# projeto e pasta emsdk ao lado do projeto (uma pasta acima).

$ErrorActionPreference = "Stop"
$raiz  = $PSScriptRoot

$candidatos = @(
    $env:EMSDK,
    (Join-Path $raiz "emsdk"),
    (Join-Path (Split-Path $raiz -Parent) "emsdk")
) | Where-Object { $_ }

$emsdk = $candidatos | Where-Object { Test-Path (Join-Path $_ "emsdk_env.ps1") } | Select-Object -First 1
if (-not $emsdk) {
    throw "emsdk não encontrado. Procurei em: $($candidatos -join ', '). Defina a variável EMSDK com a pasta do emsdk."
}
Write-Host "Usando emsdk em: $emsdk"

# Carrega o ambiente do Emscripten nesta janela (coloca o em++ no PATH)
& (Join-Path $emsdk "emsdk_env.ps1") | Out-Null

$saida = Join-Path $raiz "interface-xadrez\src\wasm"
New-Item -ItemType Directory -Force $saida | Out-Null

# O PowerShell não expande "*.cpp" para programas externos, então listamos os arquivos aqui.
$fontes = Get-ChildItem (Join-Path $raiz "engine\src") -Recurse -Filter *.cpp | ForEach-Object { $_.FullName }
$fontes += Join-Path $raiz "engine\bindings\bindings.cpp"

em++ @fontes `
    -I (Join-Path $raiz "engine\include") `
    -std=c++20 -O2 --bind `
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web `
    -o (Join-Path $saida "chess_engine.js")

if ($LASTEXITCODE -ne 0) { throw "Falha ao compilar o WebAssembly." }
Write-Host "Build OK -> $saida"
