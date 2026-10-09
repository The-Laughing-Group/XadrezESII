#pragma once
#include <cstddef>
#include "utils/constantes.hpp"
#include "lances.hpp" //Pode estar desatualizado, procurar lances.hpp

// Tipos de limite guardados na TT (resultado do alpha-beta).
enum class TTFlag : uint8_t {
    EXACT,       // valor exato (dentro da janela alpha-beta)
    LOWERBOUND,  // houve corte beta: o valor real é >= pontuacao
    UPPERBOUND   // nenhum lance superou alpha: o valor real é <= pontuacao
};

struct TTEntry {
    uint64_t chave        = 0;                // chave Zobrist (confirma que é a mesma posição)
    Lance    melhor_lance = SEM_LANCE;        // melhor lance encontrado (para ordenação)
    int16_t  pontuacao    = 0;                // avaliação, já normalizada para o nó (ver ply)
    uint8_t  profundidade = 0;                // profundidade restante com que foi buscada
    TTFlag   flag         = TTFlag::EXACT;    // tipo de limite
    uint8_t  idade        = 0;                // geração da busca que gravou a entrada
    // 1 byte de padding automático (total: 16 bytes)
};

static_assert(sizeof(TTEntry) == 16, "TTEntry deve ter 16 bytes");

// Aloca a tabela com cerca de 'megabytes' MB (o número de entradas vira potência de 2).
void iniciar_tt(std::size_t megabytes);

// Apaga todas as entradas e zera a geração. Use ao começar uma partida nova.
void limpar_tt();

// Chame uma vez no início de cada busca (a cada lance que a engine vai jogar).
// Aumenta a geração atual; entradas de gerações antigas passam a ser substituíveis.
void nova_busca_tt();

// Procura a posição na TT.
//  - Se a chave bate, 'melhor_lance' é sempre preenchido (SEM_LANCE se a entrada não tem lance).
//  - Retorna true (e preenche 'pontuacao') só se a entrada permite cortar a busca agora:
//    profundidade da entrada >= 'profundidade' pedida, e
//      EXACT                                 -> usa a pontuação,
//      LOWERBOUND e pontuacao >= beta        -> corte,
//      UPPERBOUND e pontuacao <= alpha       -> corte.
//  - 'ply' = distância da raiz da busca; usado para converter pontuações de mate.
bool ler_tt(uint64_t chave, int profundidade, int ply, int alpha, int beta,
            int& pontuacao, Lance& melhor_lance);

// Grava o resultado de um nó. 'pontuacao' vem relativa à raiz; a conversão para
// "relativa ao nó" (usando 'ply') acontece dentro de gravar_tt.
void gravar_tt(uint64_t chave, int profundidade, int ply, int pontuacao,
               TTFlag flag, Lance melhor_lance);

// ---------------------------------------------------------------------------
// Conversão de pontuações de mate (uso interno: chamadas dentro de gravar_tt/ler_tt).
// Mates dependem da distância da raiz; a TT guarda "mate em N a partir DESTE nó".
// ---------------------------------------------------------------------------
constexpr int score_para_tt(int score, int ply) {
    if (score >=  SCORE_MATE_MIN) return score + ply;
    if (score <= -SCORE_MATE_MIN) return score - ply;
    return score;
}

constexpr int score_da_tt(int score, int ply) {
    if (score >=  SCORE_MATE_MIN) return score - ply;
    if (score <= -SCORE_MATE_MIN) return score + ply;
    return score;
}