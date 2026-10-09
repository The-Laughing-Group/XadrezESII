#include "avaliacao.hpp"
#include "bitboard.hpp"
#include "lances.hpp"
#include "movimentos.hpp"
#include "tabuleiro.hpp"
#include "tipos.hpp"
#include "tt.hpp"
#include"zobrist.hpp"
#include <iostream>

//g++ -std=c++20 -Wall -Wextra -fsyntax-only teste_headers.cpp

static_assert(sizeof(Lance) == 2);
static_assert(sizeof(TTEntry) == 16);
//static_assert(criar_lance(e2, e4, LANCE_AVANCO_DUPLO) != LANCE_NULO);
static_assert(origem_do_lance(criar_lance(e2, e4, LANCE_NORMAL)) == e2);
static_assert(!eh_roque(criar_lance(a7, a8, LANCE_PROMO_TORRE)));   // o bug antigo

int main() {
    Bitboard pecas = (1ULL << 0) | (1ULL << 7) | (1ULL << 63) | get_mascara_linha(Linha::LINHA_3)
                    | get_mascara_coluna(Coluna::COLUNA_F);

    std::cout << "Visualizacao padrao:";
    BitboardUtils::imprimir_bitboard(pecas);

    return 0;
}