#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"

TEST_P(LancesExatos, GeraExatamenteOsLancesLegais) {
    const CasoLances& c = GetParam();
    EXPECT_EQ(ordenada(lancesDe(c.fen, c.casa)), ordenada(c.esperado))
        << "FEN: " << c.fen << "  casa: " << c.casa;
}

TEST_P(LancesExatos, EspelharAsCoresDaOsMesmosLances) {
    const CasoLances& c = GetParam();
    Lista espelhados;
    for (const auto& m : c.esperado) espelhados.push_back(espelharLance(m));
    const std::string fenEspelhado = espelharFen(c.fen);
    EXPECT_EQ(ordenada(lancesDe(fenEspelhado, espelharCasa(c.casa))), ordenada(espelhados))
        << "FEN espelhado: " << fenEspelhado << "  casa: " << espelharCasa(c.casa);
}