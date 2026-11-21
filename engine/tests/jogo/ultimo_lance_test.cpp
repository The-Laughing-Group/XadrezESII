#include <gtest/gtest.h>
#include <string>
#include "xadrez/jogo.hpp"
#include "tests/helpers_comuns.hpp" 

using namespace xadrez;

// ultimoLance(): o que aconteceu no lance (vem das flags do Lance)
// =====================================================================


TEST(UltimoLance, VazioAntesDoPrimeiroLance) {
    Jogo j;
    EXPECT_EQ(j.ultimoLance().uci, "");
    EXPECT_EQ(j.ultimoLance().tipo, "");
    EXPECT_FALSE(j.ultimoLance().captura);
}

TEST(UltimoLance, LanceNormalEAvancoDuplo) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("g1f3"));
    EXPECT_EQ(j.ultimoLance().uci, "g1f3");
    EXPECT_EQ(j.ultimoLance().tipo, "normal");
    EXPECT_FALSE(j.ultimoLance().captura);

    ASSERT_TRUE(j.tentarLance("e7e5"));
    EXPECT_EQ(j.ultimoLance().tipo, "avanco_duplo");
}

TEST(UltimoLance, CapturaComum) {
    Jogo j = carrega("4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1");
    ASSERT_TRUE(j.tentarLance("e4d5"));
    EXPECT_EQ(j.ultimoLance().tipo, "captura");
    EXPECT_TRUE(j.ultimoLance().captura);
    EXPECT_EQ(j.ultimoLance().pecaCapturada, "p");
    EXPECT_EQ(j.ultimoLance().casaCapturada, "d5");   // captura comum: é o próprio destino
}

TEST(UltimoLance, RoqueCurtoInformaAMovimentacaoDaTorre) {
    Jogo j = carrega("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    ASSERT_TRUE(j.tentarLance("e1g1"));
    EXPECT_EQ(j.ultimoLance().tipo, "roque_rei");
    EXPECT_EQ(j.ultimoLance().torreOrigem, "h1");
    EXPECT_EQ(j.ultimoLance().torreDestino, "f1");
    EXPECT_FALSE(j.ultimoLance().captura);
}

TEST(UltimoLance, RoqueLongoDasPretas) {
    Jogo j = carrega("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    ASSERT_TRUE(j.tentarLance("e8c8"));
    EXPECT_EQ(j.ultimoLance().tipo, "roque_dama");
    EXPECT_EQ(j.ultimoLance().torreOrigem, "a8");
    EXPECT_EQ(j.ultimoLance().torreDestino, "d8");
}

TEST(UltimoLance, RoqueCurtoDasPretasELongoDasBrancas) {
    Jogo a = carrega("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    ASSERT_TRUE(a.tentarLance("e8g8"));
    EXPECT_EQ(a.ultimoLance().torreOrigem, "h8");
    EXPECT_EQ(a.ultimoLance().torreDestino, "f8");

    Jogo b = carrega("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    ASSERT_TRUE(b.tentarLance("e1c1"));
    EXPECT_EQ(b.ultimoLance().torreOrigem, "a1");
    EXPECT_EQ(b.ultimoLance().torreDestino, "d1");
}

TEST(UltimoLance, EnPassantInformaACasaDoPeaoCapturado) {
    // peão branco e5 captura o peão preto d5 "de passagem", pousando em d6
    Jogo j = carrega("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
    ASSERT_TRUE(j.tentarLance("e5d6"));
    EXPECT_EQ(j.ultimoLance().tipo, "en_passant");
    EXPECT_TRUE(j.ultimoLance().captura);
    EXPECT_EQ(j.ultimoLance().casaCapturada, "d5");    // NÃO é d6
    EXPECT_EQ(j.ultimoLance().pecaCapturada, "p");
}

TEST(UltimoLance, EnPassantDasPretas) {
    Jogo j = carrega("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1");
    ASSERT_TRUE(j.tentarLance("d4e3"));
    EXPECT_EQ(j.ultimoLance().tipo, "en_passant");
    EXPECT_EQ(j.ultimoLance().casaCapturada, "e4");
    EXPECT_EQ(j.ultimoLance().pecaCapturada, "P");
}

TEST(UltimoLance, PromocaoESuaVersaoComCaptura) {
    Jogo a = carrega("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
    ASSERT_TRUE(a.tentarLance("a7a8q"));
    EXPECT_EQ(a.ultimoLance().tipo, "promocao");
    EXPECT_EQ(a.ultimoLance().promocao, "q");
    EXPECT_FALSE(a.ultimoLance().captura);

    Jogo b = carrega("1r2k3/P7/8/8/8/8/8/4K3 w - - 0 1");
    ASSERT_TRUE(b.tentarLance("a7b8n"));
    EXPECT_EQ(b.ultimoLance().tipo, "captura_promocao");
    EXPECT_EQ(b.ultimoLance().promocao, "n");
    EXPECT_EQ(b.ultimoLance().pecaCapturada, "r");
    EXPECT_EQ(b.ultimoLance().casaCapturada, "b8");
}

TEST(UltimoLance, LanceRecusadoNaoMudaAInformacao) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    EXPECT_FALSE(j.tentarLance("e2e4"));     // não há mais peão em e2
    EXPECT_FALSE(j.tentarLance("a1a5"));     // torre bloqueada
    EXPECT_EQ(j.ultimoLance().uci, "e2e4");
}

TEST(UltimoLance, CarregarFenLimpa) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    ASSERT_TRUE(j.carregarFen(FEN_INICIAL));
    EXPECT_EQ(j.ultimoLance().uci, "");
    EXPECT_FALSE(j.carregarFen("isto nao e um fen"));
}
