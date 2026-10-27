#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace xadrez;

// Carrega o FEN num Jogo (falha o teste se o FEN for rejeitado).
static Jogo carrega(const char* fen) {
    Jogo j;
    EXPECT_TRUE(j.carregarFen(fen)) << "FEN rejeitado: " << fen;
    return j;
}

static bool contem(const std::vector<std::string>& v, const std::string& item) {
    return std::find(v.begin(), v.end(), item) != v.end();
}

// Total de lances legais do lado que tem a vez.
static int totalDeLances(const Jogo& j) {
    int total = 0;
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r)
            total += static_cast<int>(j.lancesDe(std::string{c, r}).size());
    return total;
}

// ---------- xeque-mate ----------

TEST(FimDePartida, DetectaXequeMate) {
    const char* mates[] = {
        // mate do louco: 1.f3 e5 2.g4 Dh4#  (brancas mateadas)
        "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3",
        // mate da fileira de trás: Ta8#  (pretas mateadas)
        "R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1",
        // mate pastor: Dxf7#  (pretas mateadas)
        "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4",
        // dama protegida pelo rei, ao lado do rei adversário (pretas mateadas)
        "4k3/4Q3/4K3/8/8/8/8/8 b - - 0 1",
        // duas damas pretas se protegendo (brancas mateadas)
        "6k1/8/8/8/8/5q2/6q1/7K w - - 0 1",
    };
    for (const char* fen : mates) {
        Jogo j = carrega(fen);
        EXPECT_TRUE(j.emXeque())      << fen;
        EXPECT_TRUE(j.emXequeMate())  << fen;
        EXPECT_FALSE(j.emAfogamento()) << fen;
        EXPECT_EQ(totalDeLances(j), 0) << "um mate não pode ter lance legal: " << fen;
    }
}

// ---------- xeque que NÃO é mate (o rei tem uma saída) ----------

TEST(FimDePartida, XequeEmQueOReiFogeNaoEMate) {
    Jogo j = carrega("4k3/8/8/8/8/8/4R3/4K3 b - - 0 1");
    EXPECT_TRUE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_FALSE(j.emAfogamento());
    EXPECT_TRUE(contem(j.lancesDe("e8"), "e8d8"));
}

TEST(FimDePartida, XequeQuePodeSerBloqueadoNaoEMate) {
    // torre preta em a1 dá xeque pela primeira fileira; Td2-d1 bloqueia
    Jogo j = carrega("4k3/8/8/8/8/8/3R4/r3K3 w - - 0 1");
    EXPECT_TRUE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_TRUE(contem(j.lancesDe("d2"), "d2d1"));
}

TEST(FimDePartida, XequeQuePodeCapturarAPecaAtacanteComOReiNaoEMate) {
    // dama branca desprotegida ao lado do rei: Rxe7
    Jogo j = carrega("4k3/4Q3/8/8/8/8/8/4K3 b - - 0 1");
    EXPECT_TRUE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_TRUE(contem(j.lancesDe("e8"), "e8e7"));
}

TEST(FimDePartida, XequeQuePodeCapturarAPecaAtacanteComOutraPecaNaoEMate) {
    // dama preta em e2 protegida pelo bispo g4: o rei não pode capturar, mas Cc3xe2 pode
    Jogo j = carrega("4k3/8/8/8/6b1/2N5/4q3/4K3 w - - 0 1");
    EXPECT_TRUE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_FALSE(contem(j.lancesDe("e1"), "e1e2"));
    EXPECT_TRUE(contem(j.lancesDe("c3"), "c3e2"));
}

// ---------- afogamento ----------

TEST(FimDePartida, DetectaAfogamento) {
    Jogo j = carrega("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    EXPECT_FALSE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_TRUE(j.emAfogamento());
    EXPECT_EQ(totalDeLances(j), 0);
}

// ---------- jogo normal: nada de fim de partida ----------

TEST(FimDePartida, PosicaoInicialNaoTemFimDePartida) {
    Jogo j;
    EXPECT_FALSE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_FALSE(j.emAfogamento());
}

TEST(FimDePartida, PosicaoDeAberturaComumNaoTemFimDePartida) {
    Jogo j = carrega("rnbqkbnr/ppp2ppp/8/3pp3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3");
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

// ---------- partidas jogadas lance a lance ----------

TEST(FimDePartida, MateDoLouco_JogadoPeloJogo) {
    Jogo j;
    for (const char* lance : {"f2f3", "e7e5", "g2g4"}) {
        ASSERT_TRUE(j.tentarLance(lance)) << lance;
        EXPECT_FALSE(j.emXequeMate()) << "ainda não é mate depois de " << lance;
    }
    ASSERT_TRUE(j.tentarLance("d8h4"));

    EXPECT_TRUE(j.emXeque());
    EXPECT_TRUE(j.emXequeMate());
    EXPECT_FALSE(j.emAfogamento());
    EXPECT_TRUE(j.vezDasBrancas());   // vez das brancas + mate = pretas venceram

    // depois do mate nenhum lance é aceito
    const std::string antes = j.fen();
    EXPECT_FALSE(j.tentarLance("e2e4"));
    EXPECT_FALSE(j.tentarLance("e1f2"));
    EXPECT_EQ(j.fen(), antes);
}

TEST(FimDePartida, MatePastor_JogadoPeloJogo) {
    Jogo j;
    for (const char* lance : {"e2e4", "e7e5", "f1c4", "b8c6", "d1h5", "g8f6"}) {
        ASSERT_TRUE(j.tentarLance(lance)) << lance;
        EXPECT_FALSE(j.emXequeMate()) << lance;
    }
    ASSERT_TRUE(j.tentarLance("h5f7"));   // Dxf7#

    EXPECT_TRUE(j.emXequeMate());
    EXPECT_FALSE(j.vezDasBrancas());      // vez das pretas + mate = brancas venceram
}
