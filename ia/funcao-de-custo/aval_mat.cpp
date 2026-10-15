#include "aval_mat.hpp"

int avaliar_material(const Tabuleiro& tab) {
  int score = 0;
  for (int tipo = 0; tipo < NUM_TIPOS; ++tipo) {
    TipoPeca tpeca = static_cast<TipoPeca>(tipo);
    score += VALOR_PECA[tipo] *
    quantidade_de(tab, Cor::BRANCO, tpeca);

    score -= VALOR_PECA[tipo] *
    quantidade_de(tab, Cor::PRETO, tpeca);
  }
  return score;
}