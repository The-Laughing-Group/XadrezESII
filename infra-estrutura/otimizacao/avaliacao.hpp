#pragma once
#include "utils/tipos.hpp"   // Tabuleiro, idx(); traz também constantes.hpp (VALOR_PECA, SCORE_*)
#include "utils/constantes.hpp"

// Estrutura para avaliação em fases (tapered eval: meio-jogo vs final).
// Ainda não é usada; remova ou complete quando decidir se vai usá-la.
struct Score {
    int midgame;
    int endgame;
};

// Tabelas peça-casa (Piece-Square Tables): bônus/penalidade por casa, para cada tipo de peça.
// Indexar por PST[idx(tpeca)][casa].
//
// ATENÇÃO à orientação: com a1 = índice 0, uma tabela escrita "visualmente" (linha 8 em
// cima) fica de cabeça para baixo. As tabelas valem para as BRANCAS; para as PRETAS
// espelhe a casa: PST[idx(tpeca)][casa ^ 56].
//
// A definição fica em avaliacao.cpp. Esse .cpp deve incluir este header ANTES de definir
// a tabela, para ela ter ligação externa (extern).
extern const int PST[NUM_TIPOS][NUM_CASAS];

// Avaliação estática da posição, do ponto de vista de quem joga.
// (Positivo = bom para quem joga, negativo = ruim.)
int avaliar(const Tabuleiro& b);