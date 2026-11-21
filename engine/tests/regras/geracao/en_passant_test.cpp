#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"

static const std::vector<CasoLances> kEnPassant= {
    {"EnPassantEhGerado",
     "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5",
     {"e5d6", "e5e6"}},
    {"SemCasaDePassantNaoHaCapturaNaCasaVazia",
     "4k3/8/8/3pP3/8/8/8/4K3 w - - 0 1", "e5",
     {"e5e6"}},
    {"EnPassantDasPretas",
     "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1", "d4",
     {"d4d3", "d4e3"}},
    {"EnPassantNaColunaA",
     "4k3/8/8/Pp6/8/8/8/4K3 w - b6 0 1", "a5",
     {"a5a6", "a5b6"}},
    {"EnPassantNaColunaH",
     "4k3/8/8/6pP/8/8/8/4K3 w - g6 0 1", "h5",
     {"h5g6", "h5h6"}},
    {"EnPassantExigePeaoAdjacente",
     "4k3/8/8/3p1P2/8/8/8/4K3 w - d6 0 1", "f5",
     {"f5f6"}},
};
INSTANTIATE_TEST_SUITE_P(EnPassant, LancesExatos, ::testing::ValuesIn(kEnPassant), nomeDoCaso);