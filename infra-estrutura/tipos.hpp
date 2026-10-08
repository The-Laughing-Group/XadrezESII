#pragma once
#include <cstdint>
#include "constantes.hpp"

// Mapeamento das casas: LERF (Little-Endian Rank-File)
//   casa = 8 * linha + coluna   ->   a1 = 0, b1 = 1, ..., h1 = 7, a2 = 8, ..., h8 = 63

/* Estrutura do bitboard:
64 bits; o bit mais à esquerda (mais significativo) é h8 e o mais à direita é a1.

h8 g8 f8 .......... c8 b8 a8
.                         .
.                         .
.                         .
.                         .
h1 g1 f1 .......... c1 b1 a1

*/

// ---------------------------------------------------------------------------
// Cor e TipoPeca são "enum class": o compilador não deixa misturar tipos por
// engano (ex.: BRANCO == PEAO) e não converte para inteiro sozinho.
// Para usar como índice de array, use idx(): pecas[idx(cor)][idx(tpeca)].
// ---------------------------------------------------------------------------
enum class Cor : uint8_t {
    BRANCO,
    PRETO,
    COR_VAZIA   // "sem cor"; também vale NUM_CORES
};

constexpr int idx(Cor c) { return static_cast<int>(c); }

enum class TipoPeca : uint8_t {
    PEAO,
    CAVALO,
    BISPO,
    TORRE,
    DAMA,
    REI,
    PECA_VAZIA  // "sem peça"; também vale NUM_TIPOS
};

constexpr int idx(TipoPeca p) { return static_cast<int>(p); }

static_assert(idx(Cor::COR_VAZIA) == NUM_CORES, "COR_VAZIA deve valer NUM_CORES");
static_assert(idx(TipoPeca::PECA_VAZIA) == NUM_TIPOS, "PECA_VAZIA deve valer NUM_TIPOS");

// ---------------------------------------------------------------------------
// Casas, colunas e linhas são "enum" simples: convertem para inteiro sozinhos,
// o que facilita o uso em operações de bits e índices.
// Cada enumerador vale o anterior + 1, começando em 0 (a1 = 0 ... h8 = 63).
// ---------------------------------------------------------------------------
enum Quadrado : uint8_t {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,

    quadrado_vazio = NUM_CASAS   // "nenhuma casa" (64)
};

enum Coluna : uint8_t {
    COLUNA_A, COLUNA_B, COLUNA_C, COLUNA_D, COLUNA_E, COLUNA_F, COLUNA_G, COLUNA_H,
    COLUNA_VAZIA = NUM_COLUNAS
};

enum Linha : uint8_t {
    LINHA_1, LINHA_2, LINHA_3, LINHA_4, LINHA_5, LINHA_6, LINHA_7, LINHA_8,
    LINHA_VAZIA = NUM_LINHAS
};

using Bitboard = uint64_t;

// Máscara de uma coluna / linha inteira (as tabelas ficam em constantes.hpp).
constexpr Bitboard get_mascara_coluna(Coluna c) { return MASCARA_COLUNAS[c]; }
constexpr Bitboard get_mascara_linha(Linha l)   { return MASCARA_LINHAS[l]; }

// ---------------------------------------------------------------------------
// Estado da partida
// ---------------------------------------------------------------------------
enum DireitoRoque : uint8_t { // Ocupa somente 4 bits
    SEM_ROQUE         = 0,    // 0000
    ROQUE_BRANCO_REI  = 1,    // 0001
    ROQUE_BRANCO_DAMA = 2,    // 0010
    ROQUE_PRETO_REI   = 4,    // 0100
    ROQUE_PRETO_DAMA  = 8,    // 1000
    TODOS_ROQUES      = 15    // 1111 (posição inicial)
};

// Informações que NÃO dá para recalcular a partir das peças. Por isso cada lance
// empilha uma cópia do Estado anterior (ver Tabuleiro::historico).
// Os campos estão ordenados do maior para o menor, o que deixa o struct com 16 bytes.
struct Estado {
    uint64_t chave_zobrist = 0ULL;   // chave de hash da posição
    int      contador_50   = 0;      // meios-lances sem captura nem lance de peão
                                     // (50 lances de cada lado = 100 meios-lances)
    uint8_t  direitos_roque = TODOS_ROQUES;
    Quadrado en_passant     = Quadrado::quadrado_vazio; // casa "pulada" pelo peão. Se ele foi de e2 para e4,
                                              // guardamos e3. Sempre preenchida após um avanço duplo,
                                              // mesmo sem peão inimigo ao lado (é o que a FEN faz).
    Cor      lado_a_jogar   = Cor::BRANCO;
    TipoPeca capturada      = TipoPeca::PECA_VAZIA; // peça capturada pelo lance que levou a este
                                                    // estado (necessária para desfazer o lance)
};

static_assert(sizeof(Estado) == 16, "Estado deve ter 16 bytes");

struct Peca {
    Cor      cor   = Cor::COR_VAZIA;
    TipoPeca tpeca = TipoPeca::PECA_VAZIA;   // PECA_VAZIA = casa sem peça
};

// ---------------------------------------------------------------------------
// Tabuleiro (~33 KB): NUNCA passar por valor, sempre por referência.
// Só as funções colocar_peca / remover_peca / mover_peca (tabuleiro.hpp) devem
// alterar bitboards, piece-list e mailbox, para as três estruturas não divergirem.
// ---------------------------------------------------------------------------
struct Tabuleiro {
    // Bitboards: um para cada (cor, tipo de peça)
    Bitboard pecas[NUM_CORES][NUM_TIPOS] = {};

    // Ocupações: todas as peças de uma cor, e todas as peças do tabuleiro
    Bitboard ocupacoes[NUM_CORES] = {};
    Bitboard ocupacoes_totais = 0ULL;

    // Piece-list: casas de cada peça por (cor, tipo). Só os primeiros
    // quantidade[cor][tipo] elementos são válidos.
    // MAX_PECAS_TIPO = 10: 8 peões, ou 2 + 8 promoções.
    Quadrado lista[NUM_CORES][NUM_TIPOS][MAX_PECAS_TIPO] = {};
    int      quantidade[NUM_CORES][NUM_TIPOS] = {};

    // Mailbox: o que há em cada casa (para peca_em ser instantâneo)
    Peca casas[NUM_CASAS] = {};

    // Estado atual (direitos de roque, en passant, vez de jogar, etc.)
    Estado estado;

    // Pilha de estados anteriores (para desfazer_lance e detectar repetição).
    //   fazer_lance:    historico[conta_lance] = estado; conta_lance++;
    //   desfazer_lance: conta_lance--; estado = historico[conta_lance];
    Estado historico[TAMANHO_HISTORICO];
    int    conta_lance = 0;
};

// ---------------------------------------------------------------------------
// Funções auxiliares
// ---------------------------------------------------------------------------
constexpr Cor oposta(Cor c) { return c == Cor::BRANCO ? Cor::PRETO : Cor::BRANCO; }
constexpr Quadrado criar_quadrado(Coluna c, Linha l) { return static_cast<Quadrado>(l * 8 + c); }
constexpr Coluna coluna_de(Quadrado q) { return static_cast<Coluna>(q & 7); }
constexpr Linha  linha_de(Quadrado q)  { return static_cast<Linha>(q >> 3); }