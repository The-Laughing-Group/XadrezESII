#include "movimentos.hpp"
#include "utils/tipos.hpp"
#include "bitboard/bitboard.hpp"

static Bitboard RAIO_NORTE[64], RAIO_SUL[64], RAIO_LESTE[64], RAIO_OESTE[64];
static Bitboard RAIO_NORDESTE[64], RAIO_NOROESTE[64], RAIO_SUDESTE[64], RAIO_SUDOESTE[64];

void init_raios() {
    using namespace BitboardUtils;
    for (int i = 0; i < NUM_CASAS; i++) {
        Quadrado sq = static_cast<Quadrado>(i);
        Bitboard linha         = get_mascara_linha(linha_de(sq));
        Bitboard coluna        = get_mascara_coluna(coluna_de(sq));
        Bitboard diagonal      = get_mascara_diagonal_casa(sq);
        Bitboard antidiagonal  = get_mascara_antidiagonal_casa(sq);

        RAIO_NORTE[sq]    = bits_acima(coluna, sq);        // +8
        RAIO_SUL[sq]      = bits_abaixo(coluna, sq);       // -8
        RAIO_LESTE[sq]    = bits_acima(linha, sq);         // +1
        RAIO_OESTE[sq]    = bits_abaixo(linha, sq);        // -1
        RAIO_NORDESTE[sq] = bits_acima(diagonal, sq);      // +9
        RAIO_SUDOESTE[sq] = bits_abaixo(diagonal, sq);     // -9
        RAIO_NOROESTE[sq] = bits_acima(antidiagonal, sq);  // +7
        RAIO_SUDESTE[sq]  = bits_abaixo(antidiagonal, sq); // -7
    }
}

// Direções em que o índice AUMENTA: o primeiro bloqueador é o bit mais baixo.
// Norte, Nordeste, Noroeste, Leste
// O bit 63 entra como "sentinela": se não houver bloqueador, ele é o escolhido, e como
// RAIO[63] == 0 nessas direções, o resultado é o raio inteiro (sem precisar de if).
static inline Bitboard ataque_positivo(const Bitboard raio[], Bitboard ocupacao, Quadrado sq) {
    Bitboard bloqueadores = (raio[sq] & ocupacao) | (1ULL << 63);
    return raio[sq] ^ raio[std::countr_zero(bloqueadores)];
}

// Direções em que o índice DIMINUI: o primeiro bloqueador é o bit mais alto.
// Sul, Sudeste, Sudoeste, Oeste
// Sentinela: bit 0 (RAIO[0] == 0 nessas direções).
static inline Bitboard ataque_negativo(const Bitboard raio[], Bitboard ocupacao, Quadrado sq) {
    Bitboard bloqueadores = (raio[sq] & ocupacao) | 1ULL;
    return raio[sq] ^ raio[63 - std::countl_zero(bloqueadores)];
}

Bitboard ataques_torre(Quadrado sq, Bitboard ocupacao) {
    return ataque_positivo(RAIO_NORTE, ocupacao, sq) | ataque_positivo(RAIO_LESTE, ocupacao, sq)
         | ataque_negativo(RAIO_SUL,   ocupacao, sq) | ataque_negativo(RAIO_OESTE, ocupacao, sq);
}

Bitboard ataques_bispo(Quadrado sq, Bitboard ocupacao) {
    return ataque_positivo(RAIO_NORDESTE, ocupacao, sq) | ataque_positivo(RAIO_NOROESTE, ocupacao, sq)
         | ataque_negativo(RAIO_SUDESTE,  ocupacao, sq) | ataque_negativo(RAIO_SUDOESTE, ocupacao, sq);
}

Bitboard ataques_dama(Quadrado sq, Bitboard ocupacao) {
    return ataques_torre(sq, ocupacao) | ataques_bispo(sq, ocupacao);
}