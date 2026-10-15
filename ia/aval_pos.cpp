#include <bit>
#include "tipos.hpp"
#include "piece_square_tables.hpp"

int avaliar_posicao_pecas(const Tabuleiro& tab){
  int score = 0;
  for(int tipo = 0; tipo < NUM_TIPOS; ++tipo) {
      Bitboard brancos = tab.pecas[idx(Cor::BRANCO)][tipo];
      Bitboard pretos = tab.pecas[idx(Cor::PRETO)][tipo];
      score += avaliar_tabela(brancos, TABELAS[tipo], Cor::BRANCO);
      score += avaliar_tabela(pretos, TABELAS[tipo], Cor::PRETO);
  }
  return score;
}

int avaliar_tabela(Bitboard bitboard,  const TabelaPecas& tabela, Cor cor){
  int score = 0;
  while (bitboard != 0) {
    // Conta quantos zeros existem a direita do bit ligado menos significativo
    // e me devolve a 'posição' que esse bit está localizado no bitboard
    int casa = std::countr_zero(bitboard);
    //inverte a casa para os peões pretos, a tabela é espelhada para essas peças 
    int indice = Cor::BRANCO ? casa : (casa ^ 56);
    score += tabela[indice];
    //operação desliga o bit menos significativo
    bitboard &= bitboard - 1;
  }
  // Retorna positivo para as brancas e negativo para as pretas
  return (cor == Cor::BRANCO) ? score : -score;
}