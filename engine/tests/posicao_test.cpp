#include <gtest/gtest.h>
#include <stdexcept>
#include "xadrez/posicao.hpp"

using namespace xadrez;

TEST(Casas, TextoParaMatriz) {
    int l = -1, c = -1;
    ASSERT_TRUE(textoParaCasa("a8", l, c));
    EXPECT_EQ(l, 0); EXPECT_EQ(c, 0);
    ASSERT_TRUE(textoParaCasa("h1", l, c));
    EXPECT_EQ(l, 7); EXPECT_EQ(c, 7);
    ASSERT_TRUE(textoParaCasa("e4", l, c));
    EXPECT_EQ(l, 4); EXPECT_EQ(c, 4);
}

TEST(Casas, MatrizParaTexto) {
    EXPECT_EQ(casaParaTexto(0, 0), "a8");
    EXPECT_EQ(casaParaTexto(7, 7), "h1");
    EXPECT_EQ(casaParaTexto(6, 4), "e2");
}

TEST(Casas, TextoInvalido) {
    int l, c;
    EXPECT_FALSE(textoParaCasa("i9", l, c));
    EXPECT_FALSE(textoParaCasa("e", l, c));
    EXPECT_FALSE(textoParaCasa("e44", l, c));
    EXPECT_FALSE(textoParaCasa("", l, c));
}

TEST(Fen, PosicaoInicialNaMatriz) {
    Posicao p = Posicao::inicial();
    EXPECT_EQ(p.tabuleiro[0][0], 'r');   // torre preta em a8
    EXPECT_EQ(p.tabuleiro[0][4], 'k');   // rei preto em e8
    EXPECT_EQ(p.tabuleiro[1][3], 'p');   // peão preto em d7
    EXPECT_EQ(p.tabuleiro[4][4], VAZIA); // e4 vazia
    EXPECT_EQ(p.tabuleiro[6][4], 'P');   // peão branco em e2
    EXPECT_EQ(p.tabuleiro[7][4], 'K');   // rei branco em e1
    EXPECT_TRUE(p.vezDasBrancas);
    EXPECT_TRUE(p.rocaBrancasRei && p.rocaBrancasDama && p.rocaPretasRei && p.rocaPretasDama);
    EXPECT_EQ(p.passantLinha, -1);
    EXPECT_EQ(p.meioLances, 0);
    EXPECT_EQ(p.numeroLance, 1);
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
    EXPECT_EQ(p.meioLances, 0);
    EXPECT_EQ(p.numeroLance, 1);
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
