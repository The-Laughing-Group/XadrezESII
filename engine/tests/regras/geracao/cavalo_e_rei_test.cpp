#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"


// Cavalo, rei
static const std::vector<CasoLances> kGeometriaCavalo = {
    {"CavaloNoCentro",
     "k7/8/8/8/3N4/8/8/7K w - - 0 1", "d4",
     {"d4b3", "d4b5", "d4c2", "d4c6", "d4e2", "d4e6", "d4f3", "d4f5"}},
    {"CavaloNoCanto",
     "k7/8/8/8/8/8/8/N6K w - - 0 1", "a1",
     {"a1b3", "a1c2"}},
    {"CavaloNaoPousaEmPecaPropria",
     "k7/8/2P1P3/8/3N4/8/8/7K w - - 0 1", "d4",
     {"d4b3", "d4b5", "d4c2", "d4e2", "d4f3", "d4f5"}},
    {"CavaloCapturaPecaAdversaria",
     "k7/8/4p3/8/3N4/8/8/7K w - - 0 1", "d4",
     {"d4b3", "d4b5", "d4c2", "d4c6", "d4e2", "d4e6", "d4f3", "d4f5"}},
    {"CavaloPulaPecasNoCaminho",
     "k7/8/8/2PPP3/2PNP3/2PPP3/8/7K w - - 0 1", "d4",
     {"d4b3", "d4b5", "d4c2", "d4c6", "d4e2", "d4e6", "d4f3", "d4f5"}},
};
INSTANTIATE_TEST_SUITE_P(Cavalo, LancesExatos, ::testing::ValuesIn(kGeometriaCavalo), nomeDoCaso);

static const std::vector<CasoLances> kGeometriaRei = {
    {"ReiNoCentro",
     "k7/8/8/8/4K3/8/8/8 w - - 0 1", "e4",
     {"e4d3", "e4d4", "e4d5", "e4e3", "e4e5", "e4f3", "e4f4", "e4f5"}},
    {"ReiNoCanto",
     "k7/8/8/8/8/8/8/K7 w - - 0 1", "a1",
     {"a1a2", "a1b1", "a1b2"}},
    {"ReiNaoPousaEmPecaPropria",
     "k7/8/8/4P3/4KP2/8/8/8 w - - 0 1", "e4",
     {"e4d3", "e4d4", "e4d5", "e4e3", "e4f3", "e4f5"}},
};
INSTANTIATE_TEST_SUITE_P(Rei, LancesExatos, ::testing::ValuesIn(kGeometriaRei), nomeDoCaso);