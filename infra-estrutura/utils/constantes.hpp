#pragma once
#include <cstdint>

// ============================================================================
// constantes.hpp - todas as constantes numéricas do motor.
//
// Regra: este arquivo NÃO depende de nenhum outro arquivo do projeto (só de
// <cstdint>). Assim qualquer header pode incluí-lo sem criar dependência circular.
// Constantes que dependem de tipos do projeto (ex.: SEM_LANCE, que é um Lance)
// ficam junto do tipo, no respectivo header.
// ============================================================================

// ---------- Dimensões do tabuleiro ----------
constexpr int NUM_CASAS   = 64;
constexpr int NUM_COLUNAS = 8;
constexpr int NUM_LINHAS  = 8;

// ---------- Dimensões das estruturas ----------
constexpr int NUM_CORES         = 2;
constexpr int NUM_TIPOS         = 6;     // peão, cavalo, bispo, torre, dama, rei
constexpr int MAX_PECAS_TIPO    = 10;    // 8 peões, ou 2 + 8 promoções
constexpr int TAMANHO_HISTORICO = 2048;  // máximo de meios-lances empilhados (partida + busca)
constexpr int MAX_LANCES        = 256;   // lances por posição (o máximo real conhecido é 218)

// ---------- Regras ----------
constexpr int LIMITE_REGRA_50 = 100;     // 50 lances de cada lado = 100 meios-lances

// ---------- Busca e pontuações ----------
constexpr int MAX_PLY        = 128;      // distância máxima da raiz (inclui quiescence)
constexpr int SCORE_INFINITO = 30000;    // maior que qualquer pontuação real; nunca vai para a TT
constexpr int SCORE_MATE     = 29000;    // mate em 1. Mates mais longos valem SCORE_MATE - ply
constexpr int SCORE_MATE_MIN = SCORE_MATE - MAX_PLY;  // acima disso (ou abaixo do negativo) é mate

static_assert(SCORE_MATE < SCORE_INFINITO, "SCORE_INFINITO deve superar SCORE_MATE");
static_assert(SCORE_INFINITO <= INT16_MAX, "a pontuação da TT é int16_t");

// ---------- Valores das peças (em centipeões) ----------
// Indexar com idx(TipoPeca): PEAO, CAVALO, BISPO, TORRE, DAMA, REI, PECA_VAZIA
// uso: VALOR_PECA[idx(tpeca)]
inline constexpr int VALOR_PECA[NUM_TIPOS + 1] = {100, 320, 330, 500, 900, 20000, 0};

// ---------- Máscaras de colunas e linhas (Bitboards de 64 bits) ----------
// Indexadas de 0 a 7: coluna A..H e linha 1..8 (mapeamento LERF: a1 = bit 0).
// Cada dígito hexadecimal representa 4 bits.
inline constexpr uint64_t MASCARA_COLUNAS[NUM_COLUNAS] = {
    0x0101010101010101ULL, // Coluna A
    // Ex.: o bit 0 de cada byte, ou seja, cada linha do tabuleiro vale 0000|0001:
    /*
    0000|0001
    0000|0001
    0000|0001
    0000|0001
    0000|0001
    0000|0001
    0000|0001
    0000|0001
    */
    0x0202020202020202ULL, // Coluna B
    0x0404040404040404ULL, // Coluna C
    0x0808080808080808ULL, // Coluna D
    0x1010101010101010ULL, // Coluna E
    0x2020202020202020ULL, // Coluna F
    0x4040404040404040ULL, // Coluna G
    0x8080808080808080ULL  // Coluna H
};

inline constexpr uint64_t MASCARA_LINHAS[NUM_LINHAS] = {
    0x00000000000000FFULL, // Linha 1
    // Ex.: só o byte da linha 1 fica ligado:
    /*
    0000|0000
    0000|0000
    0000|0000
    0000|0000
    0000|0000
    0000|0000
    0000|0000
    1111|1111
    */
    0x000000000000FF00ULL, // Linha 2
    0x0000000000FF0000ULL, // Linha 3
    0x00000000FF000000ULL, // Linha 4
    0x000000FF00000000ULL, // Linha 5
    0x0000FF0000000000ULL, // Linha 6
    0x00FF000000000000ULL, // Linha 7
    0xFF00000000000000ULL  // Linha 8
};

// ---------- Layout do array de chaves Zobrist (ver zobrist.hpp) ----------
constexpr int ZOB_N_PECAS = NUM_CORES * NUM_TIPOS * NUM_CASAS;  // 768: cada (cor, tipo) em cada casa
constexpr int ZOB_N_ROQUE = 16;                                 // uma por combinação dos 4 direitos
constexpr int ZOB_N_EP    = NUM_COLUNAS;                        // uma por coluna de en passant

constexpr int ZOB_PECAS = 0;
constexpr int ZOB_ROQUE = ZOB_PECAS + ZOB_N_PECAS;
constexpr int ZOB_EP    = ZOB_ROQUE + ZOB_N_ROQUE;
constexpr int ZOB_LADO  = ZOB_EP + ZOB_N_EP;                    // 1 chave: vez das pretas
constexpr int ZOB_TOTAL = ZOB_LADO + 1;                         // 793

static_assert(ZOB_TOTAL == 793, "layout das chaves Zobrist mudou");