#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"

static const std::vector<CasoLances> kGeometriaDeslizantes = {
    {"TorreLivreNoCanto",
     "7k/8/8/8/8/8/8/R6K w - - 0 1", "a1",
     {"a1a2", "a1a3", "a1a4", "a1a5", "a1a6", "a1a7", "a1a8", "a1b1", "a1c1", "a1d1", "a1e1", "a1f1", "a1g1"}},
    {"TorreParaNaPecaPropriaECapturaAdversaria",
     "7k/3p4/8/8/3R4/8/3P4/7K w - - 0 1", "d4",
     {"d4a4", "d4b4", "d4c4", "d4d3", "d4d5", "d4d6", "d4d7", "d4e4", "d4f4", "d4g4", "d4h4"}},
    {"BispoNoCentro",
     "k7/8/8/8/3B4/8/8/7K w - - 0 1", "d4",
     {"d4a1", "d4a7", "d4b2", "d4b6", "d4c3", "d4c5", "d4e3", "d4e5", "d4f2", "d4f6", "d4g1", "d4g7", "d4h8"}},
    {"BispoBloqueadoPorPecaPropria",
     "k7/8/8/4P3/3B4/2P5/8/7K w - - 0 1", "d4",
     {"d4a7", "d4b6", "d4c5", "d4e3", "d4f2", "d4g1"}},
    {"BispoParaNaCapturaEPecaPropria",
     "k7/8/8/2p5/3B4/8/1P6/7K w - - 0 1", "d4",
     {"d4c3", "d4c5", "d4e3", "d4e5", "d4f2", "d4f6", "d4g1", "d4g7", "d4h8"}},
    {"DamaNoCentro",
     "k7/8/8/8/3Q4/8/8/7K w - - 0 1", "d4",
     {"d4a1", "d4a4", "d4a7", "d4b2", "d4b4", "d4b6", "d4c3", "d4c4", "d4c5", "d4d1", "d4d2", "d4d3", "d4d5", "d4d6", "d4d7", "d4d8", "d4e3", "d4e4", "d4e5", "d4f2", "d4f4", "d4f6", "d4g1", "d4g4", "d4g7", "d4h4", "d4h8"}},
    {"DamaBloqueadaECapturando",
     "k7/8/1p6/8/3Q1P2/8/8/7K w - - 0 1", "d4",
     {"d4a1", "d4a4", "d4b2", "d4b4", "d4b6", "d4c3", "d4c4", "d4c5", "d4d1", "d4d2", "d4d3", "d4d5", "d4d6", "d4d7", "d4d8", "d4e3", "d4e4", "d4e5", "d4f2", "d4f6", "d4g1", "d4g7", "d4h8"}},
};
INSTANTIATE_TEST_SUITE_P(Deslizantes, LancesExatos, ::testing::ValuesIn(kGeometriaDeslizantes), nomeDoCaso);