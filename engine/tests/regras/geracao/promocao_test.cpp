#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"

static const std::vector<CasoLances> kPromocao = {
    {"PromocaoGeraQuatroLances",
     "k7/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7",
     {"e7e8b", "e7e8n", "e7e8q", "e7e8r"}},
    {"PromocaoComCapturaGeraOitoLances",
     "k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7",
     {"e7d8b", "e7d8n", "e7d8q", "e7d8r", "e7e8b", "e7e8n", "e7e8q", "e7e8r"}},
    {"PromocaoComFrenteBloqueadaSoCaptura",
     "k2rn3/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7",
     {"e7d8b", "e7d8n", "e7d8q", "e7d8r"}},
    {"PromocaoDoPeaoPreto",
     "4k3/8/8/8/8/8/4p3/K7 b - - 0 1", "e2",
     {"e2e1b", "e2e1n", "e2e1q", "e2e1r"}},
    {"PromocaoDoPeaoPretoComCaptura",
     "4k3/8/8/8/8/8/4p3/K2R4 b - - 0 1", "e2",
     {"e2d1b", "e2d1n", "e2d1q", "e2d1r", "e2e1b", "e2e1n", "e2e1q", "e2e1r"}},
};
INSTANTIATE_TEST_SUITE_P(Promocao, LancesExatos, ::testing::ValuesIn(kPromocao), nomeDoCaso);