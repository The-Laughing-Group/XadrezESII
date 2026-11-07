#include <gtest/gtest.h>
#include <string>
#include "xadrez/jogo.hpp"

using namespace xadrez;

// Profundidade usada em TODOS os testes. Mude aqui para ajustar a bateria.
constexpr int PROFUNDIDADE = 5;

static Jogo carrega(const char* fen) {
    Jogo j;
    EXPECT_TRUE(j.carregarFen(fen)) << "FEN rejeitado: " << fen;
    return j;
}

static std::string lanceDaIA(Jogo& j) {
    const std::string melhor = j.melhorLanceIA(PROFUNDIDADE);
    EXPECT_FALSE(melhor.empty()) << "IA nao devolveu lance em " << j.fen();
    return melhor;
}

// ===========================================================================
// MATE EM 1
// ===========================================================================

TEST(IA_Mate1, Peao) {
    Jogo j = carrega("8/8/8/6pp/6pk/6pr/5PPP/5BKB b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g3h2");
}

TEST(IA_Mate1, Cavalo) {
    Jogo j = carrega("7k/5ppr/8/8/8/5n2/4PPPP/4BKRB b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "f3h2");
}

TEST(IA_Mate1, Bispo) {
    Jogo j = carrega("7k/2b2ppp/8/8/6n1/8/5PPP/5BKB b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "c7h2");
}

TEST(IA_Mate1, Torre) {
    // Varias torres pretas
    Jogo j = carrega("7k/2r2rrr/r3r3/2r1r3/8/5r2/5PPP/rr3BKB b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "b1f1");
}

TEST(IA_Mate1, Rainha1) {
    Jogo j = carrega("6k1/2b2ppp/8/8/8/7q/5PPP/5BK1 b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "h3h2");
}

TEST(IA_Mate1, Rainha2) {
    // Varias damas pretas
    Jogo j = carrega("7k/2q3q1/q3q3/2q1q3/8/8/5PPP/5BKB b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "e5h2");
}

// ===========================================================================
// MATE EM 2
// ===========================================================================

TEST(IA_Mate2, Rainha) {
    Jogo j = carrega("5q1k/6pp/8/8/8/6nP/8/6K1 b - - 0 1");

    EXPECT_EQ(lanceDaIA(j), "f8f1");
    ASSERT_TRUE(j.tentarLance("f8f1"));

    ASSERT_TRUE(j.tentarLance("g1h2"));

    EXPECT_EQ(lanceDaIA(j), "f1f2");
    ASSERT_TRUE(j.tentarLance("f1f2"));
    EXPECT_TRUE(j.emXequeMate());
}

TEST(IA_Mate2, SacrificioTorreBispo) {
    Jogo j = carrega("2k5/1ppp4/4b3/2b5/8/4rN2/4N1PP/4RK2 b - - 0 1");

    EXPECT_EQ(lanceDaIA(j), "e3f3");
    ASSERT_TRUE(j.tentarLance("e3f3"));

    ASSERT_TRUE(j.tentarLance("g2f3"));

    EXPECT_EQ(lanceDaIA(j), "e6h3");
    ASSERT_TRUE(j.tentarLance("e6h3"));
    EXPECT_TRUE(j.emXequeMate());
}

TEST(IA_Mate2, CavaloTorre) {
    Jogo j = carrega("2k5/1ppp3R/7B/6P1/3n4/4p3/5r2/6K1 b - - 0 1");

    EXPECT_EQ(lanceDaIA(j), "d4f3");
    ASSERT_TRUE(j.tentarLance("d4f3"));

    ASSERT_TRUE(j.tentarLance("g1h1"));

    EXPECT_EQ(lanceDaIA(j), "f2h2");
    ASSERT_TRUE(j.tentarLance("f2h2"));
    EXPECT_TRUE(j.emXequeMate());
}


// ===========================================================================
// CAPTURAS EM DIAGONAL (bispo e dama) — peca preta em g5, alvos em f4 e h4
// ===========================================================================

TEST(IA_Captura, Bispo_DamaVsCavalo) {
    Jogo j = carrega("k7/pp6/8/6b1/5Q1N/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5f4");
}

TEST(IA_Captura, Bispo_TorreVsPeao) {
    Jogo j = carrega("k7/pp6/8/6b1/5R1P/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5f4");
}

TEST(IA_Captura, Dama_DamaVsTorre) {
    Jogo j = carrega("k7/pp6/8/6q1/5Q1R/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5f4");
}

TEST(IA_Captura, Dama_TorreVsBispo) {
    Jogo j = carrega("k7/pp6/8/6q1/5R1B/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5f4");
}

TEST(IA_Captura, Dama_BispoVsCavalo) {
    Jogo j = carrega("k7/pp6/8/6q1/5B1N/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5f4");
}

// ===========================================================================
// CAPTURAS EM FILEIRA/COLUNA (torre) — peca preta em g5, alvos em g4 e h5
// ===========================================================================

TEST(IA_Captura, Torre_DamaVsCavalo) {
    // Torre preta g5. Coluna g: dama branca g4. Fileira 5: cavalo branco h5.
    Jogo j = carrega("k7/pp6/8/6rN/6Q1/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5g4");
}

TEST(IA_Captura, Torre_TorreVsPeao) {
    Jogo j = carrega("k7/pp6/8/6rP/6R1/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5g4");
}

// ===========================================================================
// CAPTURAS EM L (cavalo) — peca preta em g6, alvos em f4 e h4
// ===========================================================================

TEST(IA_Captura, Cavalo_DamaVsTorre) {
    // Cavalo preto g6. Pode capturar dama branca f4 ou torre branca h4.
    Jogo j = carrega("k7/pp6/6n1/8/5Q1R/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g6f4");
}

TEST(IA_Captura, Cavalo_TorreVsBispo) {
    Jogo j = carrega("k7/pp6/6n1/8/5R1B/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g6f4");
}

// ===========================================================================
// CAPTURAS DE PEAO (diagonal, para frente) — peao preto g5, alvos f4 e h4
// ===========================================================================

TEST(IA_Captura, Peao_DamaVsTorre) {
    Jogo j = carrega("k7/pp6/8/6p1/5Q1R/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5f4");
}

TEST(IA_Captura, Peao_TorreVsBispo) {
    Jogo j = carrega("k7/pp6/8/6p1/5R1B/8/6PP/7K b - - 0 1");
    EXPECT_EQ(lanceDaIA(j), "g5f4");
}

// ===========================================================================
//  Mate em 3
// ===========================================================================

TEST(IA_Mate3, CavaloETorre) {
    Jogo j = carrega("8/2k5/1pp2p1P/p3n3/8/r7/3Q1PPK/8 b - - 0");
    ASSERT_FALSE(j.vezDasBrancas());

    EXPECT_EQ(lanceDaIA(j), "e5g4");
    ASSERT_TRUE(j.tentarLance("e5g4"));

    ASSERT_TRUE(j.tentarLance("h2g1"));   // resposta fixa das brancas

    EXPECT_EQ(lanceDaIA(j), "a3a1");
    ASSERT_TRUE(j.tentarLance("a3a1"));

    ASSERT_TRUE(j.tentarLance("d2e1"));   // resposta fixa das brancas

    EXPECT_EQ(lanceDaIA(j), "a1e1");
    ASSERT_TRUE(j.tentarLance("a1e1"));
    EXPECT_TRUE(j.emXequeMate());
}

TEST(IA_Mate3, Rainha) {
    Jogo j = carrega("1Q6/2r2kpp/1pPq4/3pp3/3Pb3/6Pp/5P2/2R2RK1 b - - 0 1");
    ASSERT_FALSE(j.vezDasBrancas());

    EXPECT_EQ(lanceDaIA(j), "h3h2");
    ASSERT_TRUE(j.tentarLance("h3h2"));

    ASSERT_TRUE(j.tentarLance("g1h2"));   // resposta fixa das brancas

    EXPECT_EQ(lanceDaIA(j), "d6h6");
    ASSERT_TRUE(j.tentarLance("d6h6"));

    ASSERT_TRUE(j.tentarLance("h2g1"));   // resposta fixa das brancas

    EXPECT_EQ(lanceDaIA(j), "h6h1");
    ASSERT_TRUE(j.tentarLance("h6h1"));
    EXPECT_TRUE(j.emXequeMate());
}
