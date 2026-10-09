#include <vector>
#include "xadrez/posicao.hpp"

namespace xadrez {

struct Movimento {
    int deLinha = 0;
    int deColuna = 0;
    int paraLinha = 0;
    int paraColuna = 0;
    char promocao = 0;   // 'q', 'r', 'b', 'n' se promove, '0' se nao
};

// Lances básicos da peça em (linha, coluna).
//
// Regras que NÃO sao consideradas: 
//  - rei em xeque
//  - roque
// 
// Regras extras consideradas:
// - en passant
//
// Devolve lista vazia se não há peça em (linha,coluna) especificada ou se for fora do tabuleiro.
std::vector<Movimento> movBasicos(const Posicao& pos, int linha, int coluna);

}