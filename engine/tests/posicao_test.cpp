#include <gtest/gtest.h>
#include <stdexcept>
#include "xadrez/posicao.hpp"
#include "tabuleiro/tabuleiro.hpp"

using namespace xadrez;

// Convenção nova (LERF): linha 0 = fileira 1, linha 7 = fileira 8.

TEST(Casas, TextoParaCasa) {
    int l = -1, c = -1;
    ASSERT_TRUE(textoParaCasa("a8", l, c));
    EXPECT_EQ(l, 7); EXPECT_EQ(c, 0);
    ASSERT_TRUE(textoParaCasa("h1", l, c));
    EXPECT_EQ(l, 0); EXPECT_EQ(c, 7);
    ASSERT_TRUE(textoParaCasa("e4", l, c));
    EXPECT_EQ(l, 3); EXPECT_EQ(c, 4);
}

TEST(Casas, CasaParaTexto) {
    EXPECT_EQ(casaParaTexto(0, 0), "a1");
    EXPECT_EQ(casaParaTexto(7, 7), "h8");
    EXPECT_EQ(casaParaTexto(1, 4), "e2");
}

TEST(Casas, TextoInvalido) {
    int l, c;
    EXPECT_FALSE(textoParaCasa("i9", l, c));
    EXPECT_FALSE(textoParaCasa("e", l, c));
    EXPECT_FALSE(textoParaCasa("e44", l, c));
    EXPECT_FALSE(textoParaCasa("", l, c));
}

TEST(Fen, PosicaoInicialNoTabuleiro) {
    Posicao p = Posicao::inicial();
    EXPECT_EQ(peca_em(p.b, Quadrado::a8).tpeca, TipoPeca::TORRE);
    EXPECT_EQ(peca_em(p.b, Quadrado::a8).cor, Cor::PRETO);
    EXPECT_EQ(peca_em(p.b, Quadrado::e8).tpeca, TipoPeca::REI);
    EXPECT_EQ(peca_em(p.b, Quadrado::d7).tpeca, TipoPeca::PEAO);
    EXPECT_EQ(peca_em(p.b, Quadrado::d7).cor, Cor::PRETO);
    EXPECT_TRUE(casa_vazia(p.b, Quadrado::e4));
    EXPECT_EQ(peca_em(p.b, Quadrado::e2).tpeca, TipoPeca::PEAO);
    EXPECT_EQ(peca_em(p.b, Quadrado::e2).cor, Cor::BRANCO);
    EXPECT_EQ(peca_em(p.b, Quadrado::e1).tpeca, TipoPeca::REI);
    EXPECT_EQ(peca_em(p.b, Quadrado::e1).cor, Cor::BRANCO);

    EXPECT_EQ(p.b.estado.lado_a_jogar, Cor::BRANCO);
    EXPECT_EQ(p.b.estado.direitos_roque, TODOS_ROQUES);
    EXPECT_EQ(p.b.estado.en_passant, Quadrado::quadrado_vazio);
    EXPECT_EQ(p.b.estado.contador_50, 0);
}

TEST(Fen, IdaEVolta) {
    const char* fens[] = {
        FEN_INICIAL,
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    };
    for (const char* fen : fens)
        EXPECT_EQ(Posicao::deFen(fen).paraFen(), fen);
}

TEST(Fen, ComCamposOpcionaisAusentes) {
    Posicao p = Posicao::deFen("4k3/8/8/8/8/8/8/4K3 w - -");
    EXPECT_EQ(p.b.estado.contador_50, 0);
    EXPECT_EQ(p.paraFen(), "4k3/8/8/8/8/8/8/4K3 w - - 0 1");
}

TEST(Fen, Invalidos) {
    EXPECT_THROW(Posicao::deFen(""), std::invalid_argument);
    EXPECT_THROW(Posicao::deFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1"), std::invalid_argument);          // faltam fileiras
    EXPECT_THROW(Posicao::deFen("rnbqkbnr/pppppppp/9/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), std::invalid_argument); // 9 casas
    EXPECT_THROW(Posicao::deFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNX w KQkq - 0 1"), std::invalid_argument); // peça X
    EXPECT_THROW(Posicao::deFen("8/8/8/8/8/8/8/8 w - - 0 1"), std::invalid_argument);                                // sem reis
    EXPECT_THROW(Posicao::deFen("4k3/8/8/8/8/8/8/4K3 x - - 0 1"), std::invalid_argument);                            // lado inválido
    EXPECT_THROW(Posicao::deFen("4k3/8/8/8/8/8/8/4K3 w Z - 0 1"), std::invalid_argument);                            // roque inválido
    EXPECT_THROW(Posicao::deFen("4k3/8/8/8/8/8/8/4K3 w - e9 0 1"), std::invalid_argument);                           // en passant inválido
}