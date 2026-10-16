#include "avaliacao.hpp"


// Função que avalia o score daquela jogada
int avaliar(const Tabuleiro& tab){
  int score = 0;

  score += avaliar_material(tab);
  score += avaliar_posicao_pecas(tab);
  // A FAZER
  // score += avaliar_estrutura_peoes(tab);
  // score += avaliar_seguranca_rei(tab);
  // 3 3. Mobilidade
  //Avalia quantas jogadas legais ou úteis as peças podem realizar. 
  //Uma posição com mais opções interessantes pode ser melhor.

  return score;
}