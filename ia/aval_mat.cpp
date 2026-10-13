#include "tipos.hpp"
#include "constantes.hpp"

int avaliar_material(const Tabuleiro& tab) {
  constexpr int VALORES[] = {
      100,  // Peão
      320,  // Cavalo
      330,  // Bispo
      500,  // Torre
      900,  // Dama
      20000     // Rei
  };

  int score = 0;
  for (int tipo = 0; tipo < NUM_TIPOS; ++tipo) {
    score += VALORES[tipo] *
    tab.quantidade[idx(Cor::BRANCO)][tipo];

    score -= VALORES[tipo] *
    tab.quantidade[idx(Cor::PRETO)][tipo];
  }
  return score
}