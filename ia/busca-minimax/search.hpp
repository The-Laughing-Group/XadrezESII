#pragma once
#include "infra-estrutura/tabuleiro/tabuleiro.hpp"
#include "infra-estrutura/lances.hpp"
#include "ia/funcao-de-custo/avaliacao.hpp"
#include "infra-estrutura/movimentos.hpp"
#include <sstream>

extern bool g_debug_busca;
extern long long g_nos;
extern std::ostringstream g_log_busca;   // texto do debug fica aqui

int minimax_alfa_beta(Tabuleiro& tabuleiro, int profundidade, int alfa, int beta, bool eh_maximizador, int ply);
Lance obter_melhor_lance(Tabuleiro& tabuleiro, int profundidade);
