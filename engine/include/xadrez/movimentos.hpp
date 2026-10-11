#pragma once
#include <vector>
#include "xadrez/posicao.hpp"

namespace xadrez {

struct Movimento {
    int deLinha = 0;
    int deColuna = 0;
    int paraLinha = 0;
    int paraColuna = 0;
    char promocao = 0;   // 'q', 'r', 'b' ou 'n' se o peão promove; o número 0 nos demais lances
};

// Lances básicos da peça em (linha, coluna).
//
// Regras que NÃO são consideradas:
//  - rei em xeque
//  - roque
//
// Regras extras consideradas:
//  - en passant
//
// Devolve lista vazia se não há peça em (linha, coluna) ou se a casa for fora do tabuleiro.
std::vector<Movimento> movBasicos(const Posicao& pos, int linha, int coluna);

// Devolve a posição que resulta de executar o lance `m` em `pos` (a posição original não muda).
// Assume que o lance é válido: quem chama é responsável por conferir.
//
// Cuida de tudo o que muda além de "tirar a peça de um lugar e pôr em outro":
//  - captura (inclusive en passant, que remove o peão capturado da casa ao lado)
//  - roque (o lance é o do REI andando duas colunas; a torre é movida junto)
//  - promoção (troca o peão pela peça indicada em m.promocao)
//  - direitos de roque, casa de en passant, contador de meio-lances e número do lance
//  - troca da vez
Posicao aplicarLance(const Posicao& pos, const Movimento& m);

}  // namespace xadrez