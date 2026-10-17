#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace xadrez;

// ATENÇÃO: Jogo::tabuleiro() devolve '-' para casa vazia (ver jogo.cpp).
// Se o front-end esperar '.', troque em jogo.cpp E aqui.

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
    EXPECT_EQ(t[2], "--------");
    EXPECT_EQ(t[5], "--------");
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

// ---------- tentarLance ----------

TEST(Jogo, TentarLanceValidoAtualizaAPosicao) {
    Jogo j;
    EXPECT_TRUE(j.tentarLance("e2e4"));
    EXPECT_EQ(j.fen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    EXPECT_FALSE(j.vezDasBrancas());
    EXPECT_EQ(j.tabuleiro()[4], "----P---");
    EXPECT_EQ(j.tabuleiro()[6], "PPPP-PPP");
}

TEST(Jogo, TentarLanceInvalidoNaoMudaNada) {
    Jogo j;
    EXPECT_FALSE(j.tentarLance("e2e5"));   // peão não anda 3 casas
    EXPECT_FALSE(j.tentarLance("e2d3"));   // diagonal sem captura
    EXPECT_FALSE(j.tentarLance("a1a3"));   // torre bloqueada pelo próprio peão
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}

TEST(Jogo, TentarLanceDeCasaVaziaOuDePecaDoAdversario) {
    Jogo j;
    EXPECT_FALSE(j.tentarLance("e4e5"));   // casa vazia
    EXPECT_FALSE(j.tentarLance("e7e5"));   // peça preta na vez das brancas
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}

TEST(Jogo, TentarLanceComTextoMalFormado) {
    Jogo j;
    EXPECT_FALSE(j.tentarLance(""));
    EXPECT_FALSE(j.tentarLance("e2"));
    EXPECT_FALSE(j.tentarLance("e2e4e4"));
    EXPECT_FALSE(j.tentarLance("z9z9"));
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}

TEST(Jogo, SequenciaDeLancesAlternaAVez) {
    Jogo j;
    EXPECT_TRUE(j.tentarLance("e2e4"));
    EXPECT_TRUE(j.tentarLance("e7e5"));
    EXPECT_TRUE(j.tentarLance("g1f3"));
    EXPECT_TRUE(j.tentarLance("b8c6"));
    EXPECT_EQ(j.fen(), "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3");
    EXPECT_FALSE(j.tentarLance("b8c6"));   // as pretas não jogam duas vezes seguidas
}

TEST(Jogo, LancesMudamDepoisDeJogar) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    EXPECT_TRUE(j.lancesDe("e2").empty());          // a casa ficou vazia
    EXPECT_TRUE(j.lancesDe("e4").empty());          // peça das brancas, mas a vez é das pretas
    EXPECT_EQ(j.lancesDe("e7").size(), 2u);         // agora as pretas podem mover
}

TEST(Jogo, PromocaoExigeAPecaEscolhida) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("k7/4P3/8/8/8/8/8/4K3 w - - 0 1"));
    EXPECT_FALSE(j.tentarLance("e7e8"));            // falta dizer em que peça promove
    EXPECT_FALSE(j.tentarLance("e7e8k"));           // peão não vira rei
    EXPECT_TRUE(j.tentarLance("e7e8Q"));            // maiúscula também é aceita
    EXPECT_EQ(j.fen(), "k3Q3/8/8/8/8/8/8/4K3 b - - 0 1");
}

TEST(Jogo, EnPassantPeloJogo) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1"));
    EXPECT_TRUE(j.tentarLance("e5d6"));
    EXPECT_EQ(j.fen(), "4k3/8/3P4/8/8/8/8/4K3 b - - 0 1");
}