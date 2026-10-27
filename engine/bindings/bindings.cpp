// Ponte C++ -> JavaScript (só entra no build WebAssembly, não no build nativo dos testes).
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace emscripten;
using xadrez::Jogo;
using xadrez::InfoLance;
using xadrez::SituacaoPartida;

// std::vector<std::string> vira um array JavaScript comum (["e2e3", "e2e4"]).
static val paraArrayJs(const std::vector<std::string>& itens) {
    val arr = val::array();
    for (std::size_t i = 0; i < itens.size(); ++i)
        arr.set(i, itens[i]);
    return arr;
}

EMSCRIPTEN_BINDINGS(xadrez) {
    // Structs que viram objetos JavaScript comuns: jogo.situacao().estado, jogo.ultimoLance().tipo...
    value_object<InfoLance>("InfoLance")
        .field("uci",           &InfoLance::uci)
        .field("tipo",          &InfoLance::tipo)
        .field("captura",       &InfoLance::captura)
        .field("pecaCapturada", &InfoLance::pecaCapturada)
        .field("casaCapturada", &InfoLance::casaCapturada)
        .field("promocao",      &InfoLance::promocao)
        .field("torreOrigem",   &InfoLance::torreOrigem)
        .field("torreDestino",  &InfoLance::torreDestino);

    value_object<SituacaoPartida>("SituacaoPartida")
        .field("estado",   &SituacaoPartida::estado)
        .field("vencedor", &SituacaoPartida::vencedor)
        .field("emXeque",  &SituacaoPartida::emXeque)
        .field("terminou", &SituacaoPartida::terminou);

    class_<Jogo>("Jogo")
        .constructor<>()
        .function("carregarFen", &Jogo::carregarFen)
        .function("fen", &Jogo::fen)
        .function("vezDasBrancas", &Jogo::vezDasBrancas)
        .function("tentarLance", &Jogo::tentarLance)
        .function("emXeque", &Jogo::emXeque)
        .function("emXequeMate", &Jogo::emXequeMate)
        .function("emAfogamento", &Jogo::emAfogamento)
        .function("situacao", &Jogo::situacao)
        .function("ultimoLance", &Jogo::ultimoLance)
        .function("tabuleiro", optional_override([](const Jogo& j) {
            return paraArrayJs(j.tabuleiro());
        }))
        .function("lancesDe", optional_override([](const Jogo& j, const std::string& casa) {
            return paraArrayJs(j.lancesDe(casa));
        }));
}