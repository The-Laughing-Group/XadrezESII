#pragma once
#include <string>
#include "xadrez/tipos.hpp"

namespace xadrez {

inline constexpr const char* FEN_INICIAL = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

//tudo o que define uma posição no xadrez
struct Posicao {
    char tabuleiro[8][8];          //tabuleiro[linha][coluna]

    bool vezDasBrancas = true;

    //direitos de roque: true = ainda pode (o rei e a torre nunca se moveram)
    bool rocaBrancasRei  = false;  // roque curto das brancas  (K)
    bool rocaBrancasDama = false;  // roque longo das brancas  (Q)
    bool rocaPretasRei   = false;  // roque curto das pretas   (k)
    bool rocaPretasDama  = false;  // roque longo das pretas   (q)

    // Casa de destino do en passant (a casa "pulada" pelo peão que andou duas).
    // Fica em -1 quando não há en passant possível.
    int passantLinha  = -1;
    int passantColuna = -1;

    int meioLances = 0;            // lances desde a última captura ou lance de peão (regra dos 50)
    int numeroLance = 1;           // começa em 1 e soma 1 depois do lance das pretas

    Posicao();                     // tabuleiro vazio

    static Posicao inicial();
    static Posicao deFen(const std::string& fen);   // lança std::invalid_argument se o FEN for inválido
    std::string paraFen() const;
};

}
