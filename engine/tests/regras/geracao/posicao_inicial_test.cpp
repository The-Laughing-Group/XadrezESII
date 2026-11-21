#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"

TEST(Movimentos, PecasPresasNaPosicaoInicialNaoTemLances) {
    Jogo j;
    for (const char* casa : {"a1", "c1", "d1", "e1", "f1", "h1"})
        EXPECT_TRUE(j.lancesDe(casa).empty()) << casa;
}

TEST(Movimentos, PosicaoInicialTemVinteLancesParaCadaLado) {
    EXPECT_EQ(totalDoLado("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), 20);
    EXPECT_EQ(totalDoLado("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"), 20);
}