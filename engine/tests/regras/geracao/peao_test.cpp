#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"


static const std::vector<CasoLances> kPeao = {
    {"PeaoBrancoNaLinhaInicialAnda1Ou2",
     "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", "e2",
     {"e2e3", "e2e4"}},
    {"PeaoPretoAndaParaBaixo",
     "4k3/4p3/8/8/8/8/8/4K3 b - - 0 1", "e7",
     {"e7e5", "e7e6"}},
    {"PeaoForaDaLinhaInicialAnda1",
     "4k3/8/8/8/8/4P3/8/4K3 w - - 0 1", "e3",
     {"e3e4"}},
    {"PeaoBloqueadoNaPrimeiraCasaNaoAnda",
     "4k3/8/8/8/8/4n3/4P3/4K3 w - - 0 1", "e2",
     {}},
    {"PeaoBloqueadoNaSegundaCasaAnda1",
     "4k3/8/8/8/4n3/8/4P3/4K3 w - - 0 1", "e2",
     {"e2e3"}},
    {"PeaoCapturaNasDiagonais",
     "4k3/8/8/3p1p2/4P3/8/8/4K3 w - - 0 1", "e4",
     {"e4d5", "e4e5", "e4f5"}},
    {"PeaoNaoCapturaParaFrenteNemPecaPropria",
     "4k3/8/8/3Pp3/4P3/8/8/4K3 w - - 0 1", "e4",
     {}},
    {"PeaoNaColunaANaoSaiDoTabuleiro",
     "4k3/8/8/1p6/P7/8/8/4K3 w - - 0 1", "a4",
     {"a4a5", "a4b5"}},
    {"PeaoNaColunaHNaoSaiDoTabuleiro",
     "4k3/8/8/6p1/7P/8/8/4K3 w - - 0 1", "h4",
     {"h4g5", "h4h5"}},
    {"PeaoPretoBloqueadoNaoAnda",
     "4k3/4p3/4N3/8/8/8/8/4K3 b - - 0 1", "e7",
     {}},
    {"PeaoPretoCapturaParaBaixo",
     "4k3/8/8/4p3/3P1P2/8/8/4K3 b - - 0 1", "e5",
     {"e5d4", "e5e4", "e5f4"}},
};
INSTANTIATE_TEST_SUITE_P(Peao, LancesExatos, ::testing::ValuesIn(kPeao), nomeDoCaso);