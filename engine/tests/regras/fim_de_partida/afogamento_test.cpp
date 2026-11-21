#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"
#include "tests/helpers_comuns.hpp" 

using Jogo = xadrez::Jogo;

TEST(Movimentos, AfogamentoNaoTemLances) {
    // rei preto em h8 sem lances e SEM estar em xeque
    EXPECT_EQ(totalDoLado("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1"), 0);
}

// ---------- afogamento ----------

TEST(FimDePartida, DetectaAfogamento) {
    Jogo j = carrega("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    EXPECT_FALSE(j.emXeque());
    EXPECT_FALSE(j.emXequeMate());
    EXPECT_TRUE(j.emAfogamento());
    EXPECT_EQ(totalDeLances(j), 0);
}