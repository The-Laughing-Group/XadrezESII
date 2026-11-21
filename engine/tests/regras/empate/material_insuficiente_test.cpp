#include <gtest/gtest.h>
#include <string>
#include "xadrez/jogo.hpp"
#include "tests/helpers_comuns.hpp" 

using Jogo = xadrez::Jogo;

// ---------- material insuficiente ----------

TEST(Situacao, MaterialInsuficiente) {
    const char* empates[] = {
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1",            // rei x rei
        "4k3/8/8/8/8/8/8/3NK3 w - - 0 1",           // rei+cavalo x rei
        "4k3/8/8/8/8/8/8/2B1K3 w - - 0 1",          // rei+bispo x rei
        "5b1k/8/8/8/8/8/8/2B1K3 w - - 0 1",         // bispos de casas escuras dos dois lados (c1 e f8)
    };
    for (const char* fen : empates) {
        Jogo j = carrega(fen);
        EXPECT_EQ(j.situacao().estado, "material_insuficiente") << fen;
        EXPECT_TRUE(j.situacao().terminou) << fen;
    }
}

TEST(Situacao, MaterialSuficienteContinuaEmAndamento) {
    const char* jogos[] = {
        "5b1k/8/8/8/8/8/8/4KB2 w - - 0 1",          // bispos de cores DIFERENTES (f1 clara, f8 escura)
        "4k3/8/8/8/8/8/8/2NNK3 w - - 0 1",          // dois cavalos: o mate é possível
        "4k3/8/8/8/8/8/8/3NK2n w - - 0 1",          // cavalo x cavalo
        "4k3/8/8/8/8/8/8/2B1K2n w - - 0 1",         // bispo x cavalo
        "4k3/8/8/8/8/8/8/3RK3 w - - 0 1",           // torre
        "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1",          // peão
        "4k3/8/8/8/8/8/8/3QK3 w - - 0 1",           // dama
    };
    for (const char* fen : jogos) {
        Jogo j = carrega(fen);
        EXPECT_EQ(j.situacao().estado, "em_andamento") << fen;
    }
}

TEST(Situacao, MaterialInsuficienteApareceDepoisDeUmaCaptura) {
    // cavalo captura o último peão preto e sobra rei+cavalo x rei
    Jogo j = carrega("4k3/8/8/8/8/2N5/8/4K3 w - - 0 1");   // já é insuficiente: confere a regra básica
    EXPECT_EQ(j.situacao().estado, "material_insuficiente");

    Jogo k = carrega("4k3/8/3p4/8/4N3/8/8/4K3 w - - 0 1");
    EXPECT_EQ(k.situacao().estado, "em_andamento");
    ASSERT_TRUE(k.tentarLance("e4d6"));                    // Cxd6 -> rei+cavalo x rei
    EXPECT_EQ(k.situacao().estado, "material_insuficiente");
}