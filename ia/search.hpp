#include "infra-estrutura\tabuleiro\tabuleiro.hpp"
#include "infra-estrutura\lances.hpp"
#include "ia\funcao-de-custo\avaliacao.hpp"
#include "infra-estrutura\movimentos.hpp"

int minimax_alfa_beta(Tabuleiro& tabuleiro, int profundidade, int alfa, int beta, bool eh_maximizador);