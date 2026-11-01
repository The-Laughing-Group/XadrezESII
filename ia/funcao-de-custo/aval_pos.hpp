#include "infra-estrutura\utils\constantes.hpp"
#include "infra-estrutura\utils\tipos.hpp"
#include "infra-estrutura\tabuleiro\tabuleiro.hpp"
#include <bit>

int avaliar_posicao_pecas(const Tabuleiro& tab);
int avaliar_tabela(Bitboard bitboard,  const TabelaPecas& tabela, Cor cor);