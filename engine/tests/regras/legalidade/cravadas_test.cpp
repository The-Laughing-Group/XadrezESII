#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"


static const std::vector<CasoLances> kLegalidadeCravadas = {
    {"PecaCravadaSoAndaNaLinhaDaCravada",
     "4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1", "e2",
     {"e2e3", "e2e4", "e2e5", "e2e6", "e2e7", "e2e8"}},
    {"EnPassantQueExpoeOReiNaFileiraEhIlegal",
     "8/8/8/8/k2Pp2Q/8/8/3K4 b - d3 0 1", "e4",
     {"e4e3"}},
    {"PeaoCravadoNaDiagonalSoCapturaOCravador",
     "4k3/8/8/8/8/2b5/3P4/4K3 w - - 0 1", "d2",
     {"d2c3"}},
};
INSTANTIATE_TEST_SUITE_P(Legalidade, LancesExatos, ::testing::ValuesIn(kLegalidadeCravadas), nomeDoCaso);

