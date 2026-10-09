#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace xadrez;

static bool contem(const std::vector<std::string>& v, const std::string& item) {
    return std::find(v.begin(), v.end(), item) != v.end();
}

TEST(Jogo, ComecaNaPosicaoInicial) {
    Jogo j;
    EXPECT_EQ(j.fen(), FEN_INICIAL);
    EXPECT_TRUE(j.vezDasBrancas());
}

TEST(Jogo, TabuleiroDevolveOitoLinhasDeOitoCaracteres) {
    Jogo j;
    std::vector<std::string> t = j.tabuleiro();
    ASSERT_EQ(t.size(), 8u);
    EXPECT_EQ(t[0], "rnbqkbnr");
    EXPECT_EQ(t[1], "pppppppp");
    EXPECT_EQ(t[2], "........");
    EXPECT_EQ(t[5], "........");
    EXPECT_EQ(t[6], "PPPPPPPP");
    EXPECT_EQ(t[7], "RNBQKBNR");
}

TEST(Jogo, LancesDoPeaoNaPosicaoInicial) {
    Jogo j;
    std::vector<std::string> v = j.lancesDe("e2");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(contem(v, "e2e3"));
    EXPECT_TRUE(contem(v, "e2e4"));
}

TEST(Jogo, LancesDoCavaloNaPosicaoInicial) {
    Jogo j;
    std::vector<std::string> v = j.lancesDe("g1");
    EXPECT_EQ(v.size(), 2u);   // e2 está ocupada por peão próprio
    EXPECT_TRUE(contem(v, "g1f3"));
    EXPECT_TRUE(contem(v, "g1h3"));
}

TEST(Jogo, PecaDeQuemNaoTemAVezNaoTemLances) {
    Jogo j;   // vez das brancas
    EXPECT_TRUE(j.lancesDe("e7").empty());
    EXPECT_TRUE(j.lancesDe("g8").empty());
}

TEST(Jogo, CasaVaziaOuInvalidaNaoTemLances) {
    Jogo j;
    EXPECT_TRUE(j.lancesDe("e4").empty());
    EXPECT_TRUE(j.lancesDe("i9").empty());
    EXPECT_TRUE(j.lancesDe("").empty());
    EXPECT_TRUE(j.lancesDe("e22").empty());
}

TEST(Jogo, PromocaoSaiComALetraNoFinal) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("k7/4P3/8/8/8/8/8/4K3 w - - 0 1"));
    std::vector<std::string> v = j.lancesDe("e7");
    EXPECT_EQ(v.size(), 4u);
    EXPECT_TRUE(contem(v, "e7e8q"));
    EXPECT_TRUE(contem(v, "e7e8r"));
    EXPECT_TRUE(contem(v, "e7e8b"));
    EXPECT_TRUE(contem(v, "e7e8n"));
}

TEST(Jogo, VezDasPretasLiberaOsLancesDasPretas) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"));
    EXPECT_FALSE(j.vezDasBrancas());
    EXPECT_EQ(j.lancesDe("e7").size(), 2u);
    EXPECT_TRUE(j.lancesDe("e4").empty());   // peça das brancas
}

TEST(Jogo, FenInvalidoDevolveFalseEMantemAPosicao) {
    Jogo j;
    EXPECT_FALSE(j.carregarFen("isso nao e um fen"));
    EXPECT_FALSE(j.carregarFen("8/8/8/8/8/8/8/8 w - - 0 1"));   // sem reis
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}
