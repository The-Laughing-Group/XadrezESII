// Ponte C++ -> JavaScript (só entra no build WebAssembly, não no build nativo dos testes).
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace emscripten;
using xadrez::Jogo;

// std::vector<std::string> vira um array JavaScript comum (["e2e3", "e2e4"]).
static val paraArrayJs(const std::vector<std::string>& itens) {
    val arr = val::array();
    for (std::size_t i = 0; i < itens.size(); ++i)
        arr.set(i, itens[i]);
    return arr;
}

EMSCRIPTEN_BINDINGS(xadrez) {
    class_<Jogo>("Jogo")
        .constructor<>()
        .function("carregarFen", &Jogo::carregarFen)
        .function("fen", &Jogo::fen)
        .function("vezDasBrancas", &Jogo::vezDasBrancas)
        .function("tabuleiro", optional_override([](const Jogo& j) {
            return paraArrayJs(j.tabuleiro());
        }))
        .function("lancesDe", optional_override([](const Jogo& j, const std::string& casa) {
            return paraArrayJs(j.lancesDe(casa));
        }));
}
