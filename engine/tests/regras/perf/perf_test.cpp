#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"

using Jogo = xadrez::Jogo;


// ---------- 5. perft ----------
// Perft conta os lances legais folha a folha até uma profundidade. Os valores são os
// publicados para as posições de referência (chessprogramming.org/Perft_Results) e
// batem com o python-chess. Pegam de uma vez erros de roque, en passant, promoção,
// cravada e xeque. Se um falhar, o EXPECT diz em que profundidade a contagem desvia:
// compare lance a lance com um motor de referência a partir da profundidade anterior.
//
// "Rapido" roda sempre. "Profundo" fica DISABLED_ (leva minutos pela API de strings);
// para rodar: --gtest_also_run_disabled_tests --gtest_filter='*PerftProfundo*'

static std::uint64_t perft(Jogo base, int prof) {
    std::uint64_t n = 0;
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r)
            for (const auto& m : base.lancesDe(std::string{c, r})) {
                if (prof == 1) { ++n; continue; }
                Jogo filho = base;
                if (!filho.tentarLance(m)) {
                    ADD_FAILURE() << "tentarLance recusou o lance gerado " << m << " em " << base.fen();
                    continue;
                }
                n += perft(filho, prof - 1);
            }
    return n;
}

struct CasoPerft {
    const char* nome;
    const char* fen;
    std::vector<std::uint64_t> esperado;   // índice 0 = profundidade 1
};

static std::string nomeDoPerft(const ::testing::TestParamInfo<CasoPerft>& info) {
    return info.param.nome;
}

static void verificarPerft(const CasoPerft& c) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen(c.fen)) << c.fen;
    for (std::size_t d = 0; d < c.esperado.size(); ++d)
        EXPECT_EQ(perft(j, static_cast<int>(d) + 1), c.esperado[d]) << c.nome << ", profundidade " << d + 1;
}

class Perft : public ::testing::TestWithParam<CasoPerft> {};
TEST_P(Perft, ContagemPorProfundidade) { verificarPerft(GetParam()); }

static const std::vector<CasoPerft> kPerftRapido = {
    {"PosicaoInicial", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {20, 400, 8902}},
    {"Kiwipete",       "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48, 2039, 97862}},
    {"Posicao3",       "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {14, 191, 2812, 43238}},
    {"Posicao4",       "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6, 264, 9467}},
    {"Posicao5",       "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {44, 1486, 62379}},
    {"Posicao6",       "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {46, 2079, 89890}},
};
INSTANTIATE_TEST_SUITE_P(Rapido, Perft, ::testing::ValuesIn(kPerftRapido), nomeDoPerft);

class PerftProfundo : public ::testing::TestWithParam<CasoPerft> {};
TEST_P(PerftProfundo, DISABLED_ContagemPorProfundidade) { verificarPerft(GetParam()); }

static const std::vector<CasoPerft> kPerftProfundo = {
    {"PosicaoInicial", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {20, 400, 8902, 197281}},
    {"Kiwipete",       "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48, 2039, 97862, 4085603}},
    {"Posicao3",       "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {14, 191, 2812, 43238, 674624}},
    {"Posicao4",       "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6, 264, 9467, 422333}},
    {"Posicao5",       "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {44, 1486, 62379, 2103487}},
    {"Posicao6",       "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {46, 2079, 89890, 3894594}},
};
INSTANTIATE_TEST_SUITE_P(Profundo, PerftProfundo, ::testing::ValuesIn(kPerftProfundo), nomeDoPerft);