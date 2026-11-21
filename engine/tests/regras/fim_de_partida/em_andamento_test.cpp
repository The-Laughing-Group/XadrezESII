
#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"
#include "tests/helpers_comuns.hpp" 

using Jogo = xadrez::Jogo;


// ---------- jogo normal: nada de fim de partida ----------
TEST(FimDePartida, PosicaoInicialNaoTemFimDePartida) {
    Jogo j;
    EXPECT_FALSE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_FALSE(j.emAfogamento());
}

TEST(FimDePartida, PosicaoDeAberturaComumNaoTemFimDePartida) {
    // 1.e4 e5 2.d4 d5: as duas cores jogaram 2 lances, então a posição é alcançável
    Jogo j = carrega("rnbqkbnr/ppp2ppp/8/3pp3/3PP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3");
    EXPECT_FALSE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_FALSE(j.emAfogamento());
}

// ---------- as consultas não podem mexer na partida ----------

TEST(FimDePartida, ConsultarNaoAlteraAPosicao) {
    const char* fens[] = {
        "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3",   // mate
        "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",                                  // afogamento
        "4k3/8/8/8/8/8/3R4/r3K3 w - - 0 1",                                // xeque defensável
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",   // roques, en passant etc.
    };
    for (const char* fen : fens) {
        Jogo j = carrega(fen);
        const std::string antes = j.fen();
        (void)j.emXeque();
        (void)j.emXequeMate();
        (void)j.emAfogamento();
        (void)j.emXequeMate();   // repetir não pode dar resultado diferente nem acumular estado
        EXPECT_EQ(j.fen(), antes) << fen;
        EXPECT_EQ(j.tabuleiro().size(), 8u);
    }
}