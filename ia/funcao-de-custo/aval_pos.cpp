#include "aval_pos.hpp"
#include <bit>
#include <algorithm>

static int calcularFase(const Tabuleiro& tab);

int avaliar_posicao_pecas(const Tabuleiro& tab){
  int score = 0;
  for(int tipo = 0; tipo < idx(TipoPeca::REI); ++tipo) {
      Bitboard brancos = tab.pecas[idx(Cor::BRANCO)][tipo];
      Bitboard pretos = tab.pecas[idx(Cor::PRETO)][tipo];
      score += avaliar_tabela(brancos, TABELAS[tipo], Cor::BRANCO);
      score += avaliar_tabela(pretos, TABELAS[tipo], Cor::PRETO);
  }

  Bitboard reisBrancos = tab.pecas[idx(Cor::BRANCO)][idx(TipoPeca::REI)];
  Bitboard reisPretos = tab.pecas[idx(Cor::PRETO)][idx(TipoPeca::REI)];

  int scoreReiMeioJogo =
    avaliar_tabela(reisBrancos, TABELA_REI_MEIO_JOGO, Cor::BRANCO) +
    avaliar_tabela(reisPretos, TABELA_REI_MEIO_JOGO, Cor::PRETO);

  int scoreReiFinal =
    avaliar_tabela(reisBrancos, TABELA_REI_FINAL, Cor::BRANCO) +
    avaliar_tabela(reisPretos, TABELA_REI_FINAL, Cor::PRETO);

  int fase = calcularFase(tab);
  int scoreRei =
    (scoreReiMeioJogo * fase + scoreReiFinal * (24 - fase)) / 24;
  return score + scoreRei;
}

int avaliar_tabela(Bitboard bitboard,  const TabelaPecas& tabela, Cor cor){
  int score = 0;
  while (bitboard != 0) {
    // Conta quantos zeros existem a direita do bit ligado menos significativo
    // e me devolve a 'posição' que esse bit está localizado no bitboard
    int casa = std::countr_zero(bitboard);
    //inverte a casa para os peões pretos, a tabela é espelhada para essas peças 
    int indice = cor == Cor::BRANCO ? casa : (casa ^ 56);
    score += tabela[indice];
    //operação desliga o bit menos significativo
    bitboard &= bitboard - 1;
  }
  // Retorna positivo para as brancas e negativo para as pretas
  return (cor == Cor::BRANCO) ? score : -score;
}

static int calcularFase(const Tabuleiro& tab) {
    int fase = 0;

    for (int cor = 0; cor < NUM_CORES; ++cor) {
        fase += 4 * std::popcount(tab.pecas[cor][idx(TipoPeca::DAMA)]);
        fase += 2 * std::popcount(tab.pecas[cor][idx(TipoPeca::TORRE)]);
        fase += std::popcount(tab.pecas[cor][idx(TipoPeca::BISPO)]);
        fase += std::popcount(tab.pecas[cor][idx(TipoPeca::CAVALO)]);
    }

    return std::min(fase, 24);
}