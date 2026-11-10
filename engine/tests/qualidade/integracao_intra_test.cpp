// engine/tests/qualidade/integracao_intra_test.cpp
// INTRA-modulo: Gerador de lances + fazer/desfazer + Zobrist + regras de fim de jogo (infra-estrutura)
#include <vector>
#include "helpers_motor.hpp"
using namespace xadrez;

namespace {
const char* kFens[] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3",
    "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",
    "k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1",
    "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1",
};
}

TEST(Integracao_Intra, GeradorMaisFazerDesfazer_PreservaTudoParaCadaLance) {
    for (const char* fen : kFens) {
        Posicao p = Posicao::deFen(fen);
        auto base = std::make_unique<Tabuleiro>(p.b);
        ListaLances lista;
        gerar_lances(p.b, lista);
        for (int i = 0; i < lista.quantidade; ++i) {
            const Lance m = lista.lances[i];
            if (fazer_lance(p.b, m)) {
                EXPECT_TRUE(verificar_consistencia(p.b)) << fen << " " << th::paraUci(m);
                EXPECT_EQ(p.b.estado.chave_zobrist, calcular_chave(p.b)) << fen << " " << th::paraUci(m);
                desfazer_lance(p.b, m);
            }
            EXPECT_TRUE(th::iguais(*base, p.b)) << fen << " lance " << th::paraUci(m);
        }
    }
}

struct CasoNos { const char* fen; std::vector<std::uint64_t> esperado; };
TEST(Integracao_Intra, ContagemDeNosBateComPerftConhecido) {
    const CasoNos casos[] = {
        {kFens[0], {20, 400, 8902}},
        {kFens[1], {48, 2039, 97862}},
        {kFens[2], {14, 191, 2812}},
        {kFens[3], {6, 264, 9467}},
        {kFens[4], {44, 1486, 62379}},
        {kFens[5], {46, 2079, 89890}},
    };
    for (const CasoNos& c : casos) {
        Posicao p = Posicao::deFen(c.fen);
        for (std::size_t d = 0; d < c.esperado.size(); ++d)
            EXPECT_EQ(th::contarNos(p.b, static_cast<int>(d) + 1), c.esperado[d]) << c.fen << " prof " << d + 1;
    }
}

TEST(Integracao_Intra, FimDeJogo_ConcordaComAContagemDeLancesLegais) {
    for (const char* fen : kFens) {
        Posicao p = Posicao::deFen(fen);
        const int legais = th::contarLegais(p.b);
        const bool xeque = em_xeque(p.b, p.b.estado.lado_a_jogar);
        EXPECT_EQ(tem_lance_legal(p.b), legais > 0) << fen;
        EXPECT_EQ(em_xeque_mate(p.b), legais == 0 && xeque) << fen;
        EXPECT_EQ(em_afogamento(p.b), legais == 0 && !xeque) << fen;
        const FimDeJogo fim = verificarFim(p.b);
        if (legais == 0) EXPECT_EQ(fim, xeque ? FimDeJogo::mate : FimDeJogo::empate) << fen;
        EXPECT_EQ(th::contarLegais(p.b), legais) << "consultas alteraram o tabuleiro: " << fen;
    }
}

TEST(Integracao_Intra, EmXequeConcordaComCasaAtacada) {
    for (const char* fen : kFens) {
        Posicao p = Posicao::deFen(fen);
        for (Cor cor : {Cor::BRANCO, Cor::PRETO})
            EXPECT_EQ(em_xeque(p.b, cor), casa_atacada(p.b, casa_do_rei(p.b, cor), oposta(cor))) << fen;
    }
}

TEST(Integracao_Intra, PartidaCompletaDeIdaEVolta_RuyLopez) {
    Posicao p = Posicao::deFen(FEN_INICIAL);
    auto inicio = std::make_unique<Tabuleiro>(p.b);
    std::vector<Lance> feitos;
    for (const char* uci : {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "b5a4", "g8f6", "e1g1", "f8e7"}) {
        const Lance m = th::acharLance(p.b, uci);
        ASSERT_NE(m, SEM_LANCE) << uci;
        ASSERT_TRUE(fazer_lance(p.b, m)) << uci;
        feitos.push_back(m);
    }
    EXPECT_EQ(th::semContadores(p.paraFen()), "r1bqk2r/1pppbppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQ1RK1 w kq -");
    EXPECT_EQ(p.b.estado.chave_zobrist, calcular_chave(p.b));
    for (auto it = feitos.rbegin(); it != feitos.rend(); ++it) desfazer_lance(p.b, *it);
    EXPECT_TRUE(th::iguais(*inicio, p.b));
}
