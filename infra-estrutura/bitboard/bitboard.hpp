#pragma once
#include <bit>        
#include "utils/tipos.hpp"

// Operações sobre bitboards. Em todas, 'casa' deve ser uma casa real (a1..h8).
// Com quadrado_vazio (64) o deslocamento "1ULL << 64" é comportamento indefinido.
// TODO: quando o assert estiver funcionando, validar com assert(casa < NUM_CASAS).

namespace BitboardUtils {

    // Bitboard com somente o bit da casa ligado (a "máscara" da casa).
    constexpr Bitboard mascara_casa(Quadrado casa) {
        return 1ULL << casa;
    }

    // Liga o bit da casa.
    inline void setar_bit(Bitboard& bb, Quadrado casa) {
        bb |= mascara_casa(casa);
    }

    // Desliga o bit da casa.
    inline void limpar_bit(Bitboard& bb, Quadrado casa) {
        bb &= ~mascara_casa(casa);
    }

    // Verifica se o bit da casa está ligado.
    inline bool testar_bit(Bitboard bb, Quadrado casa) {
        return (bb & mascara_casa(casa)) != 0;
    }

    // Inverte o bit da casa (0 vira 1, 1 vira 0).
    inline void trocar_bit(Bitboard& bb, Quadrado casa) {
        bb ^= mascara_casa(casa);
    }

    // Bitboard de uma peça (somente leitura). Para ALTERAR as peças use as funções de
    // tabuleiro.hpp (colocar_peca, remover_peca, mover_peca), que mantêm tudo sincronizado.
    inline Bitboard obter_bitboard(const Tabuleiro& b, Cor cor, TipoPeca tpeca) {
        return b.pecas[idx(cor)][idx(tpeca)];
    }

    // Imprime o bitboard como um tabuleiro 8x8 (linha 8 em cima). Definido em bitboard.cpp.
    void imprimir_bitboard(Bitboard bb);

    // Quantos bits estão ligados (ex.: contar peças).
    inline int contar_bits(Bitboard bb) {
        return std::popcount(bb);
    }

    // Índice do bit ligado menos significativo.
    // Se bb == 0, devolve 64 (quadrado_vazio).
    inline Quadrado indice_lsb(Bitboard bb) {
        return static_cast<Quadrado>(std::countr_zero(bb));
    }

    // Devolve o índice do bit menos significativo E o remove de bb.
    // É o jeito padrão de percorrer todos os bits ligados de um bitboard:
    //     while (bb) { Quadrado sq = remover_lsb(bb); ... }
    inline Quadrado remover_lsb(Bitboard& bb) {
        Quadrado sq = indice_lsb(bb);
        bb &= (bb - 1);   // desliga o LSB (truque de Brian Kernighan)
        return sq;
    }
}