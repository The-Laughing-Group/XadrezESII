#!/usr/bin/env bash
# Compila e testa o motor C++ (engine/). Funciona em Linux, macOS e Windows (Git Bash / WSL / MSYS2).
#
# Uso:  ./scripts/engine.sh [comando] [opções]
#   comandos:  test (padrão) | build | rebuild | clean
#   opções:    --release        compila em Release (padrão: Debug)
#              --filter TEXTO   roda só os testes cujo nome contém TEXTO (ex.: --filter Perft)
#              -h, --help       mostra esta ajuda
# Variáveis opcionais:
#   GENERATOR="MinGW Makefiles"   escolhe o gerador do CMake (útil no Windows sem Visual Studio)

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENGINE="$ROOT/engine"
BUILD="$ENGINE/build"
CONFIG="Debug"
CMD="test"
FILTER=""

usage() { sed -n '2,12p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; }

while [[ $# -gt 0 ]]; do
  case "$1" in
    test|build|rebuild|clean) CMD="$1" ;;
    --release) CONFIG="Release" ;;
    --filter)  shift; FILTER="${1:-}" ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Argumento desconhecido: $1" >&2; usage; exit 1 ;;
  esac
  shift
done

# ---- número de processadores ----
if command -v nproc >/dev/null 2>&1; then JOBS="$(nproc)"
elif command -v sysctl >/dev/null 2>&1; then JOBS="$(sysctl -n hw.ncpu)"
else JOBS="${NUMBER_OF_PROCESSORS:-2}"; fi

# ---- clean ----
if [[ "$CMD" == "clean" || "$CMD" == "rebuild" ]]; then
  echo ">> Limpando $BUILD"
  rm -rf "$BUILD"
  [[ "$CMD" == "clean" ]] && exit 0
  CMD="test"
fi

# ---- verifica o CMake ----
if ! command -v cmake >/dev/null 2>&1; then
  echo "ERRO: 'cmake' não encontrado." >&2
  case "$(uname -s)" in
    Linux*)  echo "Instale com: sudo apt install build-essential cmake" >&2 ;;
    Darwin*) echo "Instale com: brew install cmake  (e xcode-select --install)" >&2 ;;
    *)       echo "Windows: instale o CMake (cmake.org) e um compilador (Visual Studio Build Tools ou MinGW)," >&2
             echo "         ou use o WSL2 com: sudo apt install build-essential cmake" >&2 ;;
  esac
  exit 1
fi

# ---- configura (só se ainda não foi configurado) ----
GEN_ARGS=()
[[ -n "${GENERATOR:-}" ]] && GEN_ARGS=(-G "$GENERATOR")

if [[ ! -f "$BUILD/CMakeCache.txt" ]]; then
  echo ">> Configurando ($CONFIG)"
  cmake -S "$ENGINE" -B "$BUILD" -DCMAKE_BUILD_TYPE="$CONFIG" ${GEN_ARGS[@]+"${GEN_ARGS[@]}"}
fi

# ---- compila ----
echo ">> Compilando"
cmake --build "$BUILD" --config "$CONFIG" -j "$JOBS"
[[ "$CMD" == "build" ]] && { echo ">> Build ok"; exit 0; }

# ---- testa ----
echo ">> Rodando testes"
CTEST_ARGS=(--test-dir "$BUILD" -C "$CONFIG" --output-on-failure -j "$JOBS")
[[ -n "$FILTER" ]] && CTEST_ARGS+=(-R "$FILTER")
ctest "${CTEST_ARGS[@]}"