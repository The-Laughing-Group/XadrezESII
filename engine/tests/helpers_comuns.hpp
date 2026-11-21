#include <gtest/gtest.h>
#include <string>
#include "xadrez/jogo.hpp"

#pragma once

using Jogo = xadrez::Jogo;

inline Jogo carrega(const char* fen) {
    Jogo j;
    EXPECT_TRUE(j.carregarFen(fen)) << "FEN rejeitado: " << fen;
    return j;
}