#pragma once
#include "utils/tipos.hpp"  // traz constantes.hpp (offsets ZOB_*)

// Array único com todas as chaves aleatórias de 64 bits. Layout (offsets em constantes.hpp):
//   ZOB_PECAS ...  768 chaves: peça (cor, tipo) em cada casa -> use zob_peca()
//   ZOB_ROQUE ...   16 chaves: uma por combinação dos 4 direitos de roque
//   ZOB_EP    ...    8 chaves: uma por coluna de en passant
//   ZOB_LADO         1 chave: incluída quando é a vez das pretas
// Definido em zobrist.cpp.
extern uint64_t ZOBRIST[ZOB_TOTAL];

// Índice no array ZOBRIST da peça (cor, tipo) na casa q.
constexpr int zob_peca(Cor c, TipoPeca t, Quadrado q) {
    return (idx(c) * NUM_TIPOS + idx(t)) * NUM_CASAS + q;
}

// Uso (XOR é reversível: aplicar duas vezes desfaz):
//   mover/colocar/remover peça:  chave ^= ZOBRIST[zob_peca(cor, tpeca, casa)];
//   mudar direitos de roque:     chave ^= ZOBRIST[ZOB_ROQUE + antigos] ^ ZOBRIST[ZOB_ROQUE + novos];
//                                (remove a chave dos direitos antigos e adiciona a dos novos)
//   en passant (só se houver):   chave ^= ZOBRIST[ZOB_EP + coluna_de(en_passant)];
//                                (aplicar só se en_passant != quadrado_vazio)
//   trocar a vez de jogar:       chave ^= ZOBRIST[ZOB_LADO];  (a cada lance)

// En passant só faz parte da posição (e da chave) se o lado a jogar tem um peão que
// possa realmente capturar na casa 'ep' (FIDE 9.2.3).
// Não olha cravada: um peão cravado ainda conta (a chave só fica mais "separada", nunca junta
// posições diferentes).
inline bool ep_capturavel(const Tabuleiro& b, Quadrado ep, Cor quem_captura) {
    if (ep == Quadrado::quadrado_vazio) return false;
    const Bitboard bb = 1ULL << ep;
    // casas de onde um peão de 'quem_captura' alcança 'ep' (mesma conta de casa_atacada)
    const Bitboard origens = (quem_captura == Cor::BRANCO)
        ? (((bb >> 7) & ~MASCARA_COLUNAS[COLUNA_A]) | ((bb >> 9) & ~MASCARA_COLUNAS[COLUNA_H]))
        : (((bb << 7) & ~MASCARA_COLUNAS[COLUNA_H]) | ((bb << 9) & ~MASCARA_COLUNAS[COLUNA_A]));
    return (origens & b.pecas[idx(quem_captura)][idx(TipoPeca::PEAO)]) != 0;
}

// Sorteia os números aleatórios (semente fixa, resultados reproduzíveis).
// Deve rodar ANTES de posicao_inicial() / carregar_fen(); iniciar_motor() cuida disso.
void iniciar_zobrist();

// Calcula a chave de uma posição do zero, percorrendo tudo.
// Serve para inicializar a posição e para testar se o cálculo incremental
// (feito dentro de fazer_lance) está batendo.
uint64_t calcular_chave(const Tabuleiro& b);