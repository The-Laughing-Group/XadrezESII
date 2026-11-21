#include <gtest/gtest.h>
#include <string>
#include "xadrez/jogo.hpp"
#include "tests/helpers_comuns.hpp" 

using Jogo = xadrez::Jogo;

// ---------- regra dos 50 lances ----------

TEST(Situacao, RegraDos50Lances) {
    Jogo j = carrega("4k3/8/8/8/8/8/R7/4K3 w - - 99 80");
    EXPECT_EQ(j.situacao().estado, "em_andamento");
    ASSERT_TRUE(j.tentarLance("a2a3"));                    // lance de torre, sem captura: 99 -> 100
    EXPECT_EQ(j.situacao().estado, "regra_dos_50");
    EXPECT_TRUE(j.situacao().terminou);
    EXPECT_EQ(j.situacao().vencedor, "");
}

TEST(Situacao, LanceDePeaoOuCapturaZeraARegraDos50) {
    Jogo j = carrega("4k3/8/8/8/8/8/P6R/4K3 w - - 99 80");
    ASSERT_TRUE(j.tentarLance("a2a3"));                    // lance de peão zera o contador
    EXPECT_EQ(j.situacao().estado, "em_andamento");
}

TEST(Situacao, MateTemPrioridadeSobreARegraDos50) {
    Jogo j = carrega("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 100 3");
    EXPECT_EQ(j.situacao().estado, "xeque_mate");
    EXPECT_EQ(j.situacao().vencedor, "pretas");
}