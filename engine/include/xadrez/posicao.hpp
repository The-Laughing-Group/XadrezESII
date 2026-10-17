#pragma once
#include "tabuleiro/tabuleiro.hpp"
#include "lances.hpp"
#include <string>

namespace xadrez {
    struct Posicao {
        Tabuleiro b;

        Posicao();
        static Posicao inicial();
        static Posicao deFen(const std::string& fen);
        std::string paraFen() const;
    };

    inline constexpr const char* FEN_INICIAL =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    std::string casaParaTexto(int linha, int coluna);
    bool textoParaCasa(const std::string& texto, int& linha, int& coluna);

    Posicao aplicarLance(const Posicao& pos, Lance m);
}