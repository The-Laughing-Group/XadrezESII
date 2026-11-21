#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"

using Jogo = xadrez::Jogo;

static const std::vector<CasoLances> kLegalidadeLancesLegais = {
    {"EmXequeCavaloNaoTemLances",
     "k3r3/8/8/8/8/8/8/1N2K3 w - - 0 1", "b1",
     {}},
    {"EmXequeReiSoFoge",
     "k3r3/8/8/8/8/8/8/1N2K3 w - - 0 1", "e1",
     {"e1d1", "e1d2", "e1f1", "e1f2"}},
    {"EmXequePodeBloquear",
     "1k2r3/8/8/8/R7/8/8/4K3 w - - 0 1", "a4",
     {"a4e4"}},
    {"DuploXequeSoOReiAnda",
     "4r1k1/8/8/8/1b6/8/8/R3K1N1 w - - 0 1", "e1",
     {"e1d1", "e1f1", "e1f2"}},
    {"DuploXequeTorreNaoAnda",
     "4r1k1/8/8/8/1b6/8/8/R3K1N1 w - - 0 1", "a1",
     {}},
    {"DuploXequeCavaloNaoAnda",
     "4r1k1/8/8/8/1b6/8/8/R3K1N1 w - - 0 1", "g1",
     {}},
    {"EnPassantQueExpoeOReiNaFileiraEhIlegal",
     "8/8/8/8/k2Pp2Q/8/8/3K4 b - d3 0 1", "e4",
     {"e4e3"}},
    {"PeaoCravadoNaDiagonalSoCapturaOCravador",
     "4k3/8/8/8/8/2b5/3P4/4K3 w - - 0 1", "d2",
     {"d2c3"}},
};
INSTANTIATE_TEST_SUITE_P(Legalidade, LancesExatos, ::testing::ValuesIn(kLegalidadeLancesLegais), nomeDoCaso);

TEST(Movimentos, CasaVaziaNaoTemLances) {
    EXPECT_TRUE(lancesDe("4k3/8/8/8/8/8/8/4K3 w - - 0 1", "d4").empty());
}

TEST(Movimentos, ForaDoTabuleiroOuMalFormadoNaoTemLances) {
    Jogo j;
    EXPECT_TRUE(j.lancesDe("a0").empty());
    EXPECT_TRUE(j.lancesDe("a9").empty());
    EXPECT_TRUE(j.lancesDe("i1").empty());
    EXPECT_TRUE(j.lancesDe("").empty());
    EXPECT_TRUE(j.lancesDe("e").empty());
    EXPECT_TRUE(j.lancesDe("e22").empty());
    EXPECT_TRUE(j.lancesDe("e2e4").empty());
}