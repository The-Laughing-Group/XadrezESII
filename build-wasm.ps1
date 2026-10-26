# Compila o motor C++ para WebAssembly e gera os arquivos que o frontend importa.
# Uso (na raiz do projeto):
#   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
#   .\build-wasm.ps1
#
# O emsdk é procurado, nesta ordem: variável de ambiente EMSDK, C:\emsdk, pasta emsdk dentro do
# projeto e pasta emsdk ao lado do projeto (uma pasta acima).

$ErrorActionPreference = "Stop"
$raiz  = $PSScriptRoot

$candidatos = @(
    $env:EMSDK,
    "C:\emsdk",
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

# Fontes: as mesmas que o CMake compila em chess_core (engine/src + infra-estrutura),
# mais os bindings do WebAssembly. teste_headers.cpp é só uma verificação de headers.
$fontes = @()
$fontes += Get-ChildItem (Join-Path $raiz "engine\src") -Recurse -Filter *.cpp | ForEach-Object { $_.FullName }
$fontes += Get-ChildItem (Join-Path $raiz "infra-estrutura") -Recurse -Filter *.cpp |
           Where-Object { $_.Name -ne "teste_headers.cpp" } | ForEach-Object { $_.FullName }
$fontes += Join-Path $raiz "engine\bindings\bindings.cpp"

$incluir = @("-I", (Join-Path $raiz "engine\include"), "-I", (Join-Path $raiz "infra-estrutura"))

# Objetos ficam em build-wasm\obj (a pasta build-wasm\ já está no .gitignore).
$pastaObj = Join-Path $raiz "build-wasm\obj"
if (Test-Path $pastaObj) { Remove-Item $pastaObj -Recurse -Force }
New-Item -ItemType Directory -Force $pastaObj | Out-Null

# -fexceptions: sem ele o try/catch do Jogo::carregarFen nao funciona dentro do WebAssembly.
# Cada fonte vira um objeto separado. Há dois "movimentos.cpp" (engine e infra-estrutura): com o
# mesmo nome, um .o sobrescreveria o outro. Por isso o nome do objeto vem do caminho completo.
$objetos = @()
foreach ($fonte in $fontes) {
    $relativo = $fonte.Substring($raiz.Length).TrimStart('\', '/')
    $nome = ($relativo -replace '[\\/:]', '_') -replace '\.cpp$', '.o'
    $objeto = Join-Path $pastaObj $nome
    Write-Host "Compilando $relativo"
    em++ -c $fonte @incluir -std=c++20 -O2 -fexceptions -o $objeto
    if ($LASTEXITCODE -ne 0) { throw "Falha ao compilar $relativo." }
    $objetos += $objeto
}

Write-Host "Linkando o WebAssembly"
em++ @objetos --bind -O2 -fexceptions `
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web `
    -o (Join-Path $saida "chess_engine.js")

if ($LASTEXITCODE -ne 0) { throw "Falha ao linkar o WebAssembly." }
Write-Host "Build OK -> $saida"
