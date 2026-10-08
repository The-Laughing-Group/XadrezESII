#pragma once
#include <string>
#include "utils/tipos.hpp"

// ---------------------------------------------------------------------------
// Preparação
// ---------------------------------------------------------------------------

// Zera bitboards, ocupações, piece-list, quantidades, mailbox, histórico e
// restaura o Estado padrão.
void limpar(Tabuleiro& b);

// Coloca as 32 peças na posição inicial e calcula a chave Zobrist.
// Requer iniciar_motor() (ou ao menos iniciar_zobrist()) já chamada.
void posicao_inicial(Tabuleiro& b);

// Recalcula ocupacoes[BRANCO], ocupacoes[PRETO] e ocupacoes_totais a partir dos
// bitboards das peças. Útil para reconstruir/depurar; o uso normal fica a cargo de
// colocar_peca / remover_peca / mover_peca.
void atualizar_ocupacao(Tabuleiro& b);

// ---------------------------------------------------------------------------
// As ÚNICAS três funções que alteram as peças. Cada uma atualiza, tudo junto:
// bitboard da peça, ocupacoes, ocupacoes_totais, lista, quantidade e casas (mailbox).
// ---------------------------------------------------------------------------
void colocar_peca(Tabuleiro& b, Cor cor, TipoPeca tpeca, Quadrado sq);   // a casa deve estar vazia
void remover_peca(Tabuleiro& b, Cor cor, TipoPeca tpeca, Quadrado sq);   // a peça deve estar na casa
void mover_peca(Tabuleiro& b, Cor cor, TipoPeca tpeca, Quadrado origem, Quadrado destino);

// ---------------------------------------------------------------------------
// Consultas (somente leitura)
// ---------------------------------------------------------------------------

// Peça na casa (leitura direta do mailbox). Casa vazia: tpeca == TipoPeca::PECA_VAZIA.
Peca peca_em(const Tabuleiro& b, Quadrado sq);

bool casa_vazia(const Tabuleiro& b, Quadrado sq);

// Quantas peças daquela cor e tipo existem no tabuleiro.
int quantidade_de(const Tabuleiro& b, Cor cor, TipoPeca tpeca);

// Casa do rei da cor dada (quadrado_vazio se não houver rei).
Quadrado casa_do_rei(const Tabuleiro& b, Cor cor);

// ---------------------------------------------------------------------------
// Depuração e testes
// ---------------------------------------------------------------------------

// Confere se bitboards, piece-list e mailbox contam a mesma história.
// Devolve false (ou dispara assert) se algo estiver fora de sincronia.
bool verificar_consistencia(const Tabuleiro& b);

// Imprime o tabuleiro no terminal (P N B R Q K para as brancas, minúsculas para as pretas).
void imprimir(const Tabuleiro& b);

// Monta uma posição a partir de uma FEN (para testes). Devolve false se a FEN for inválida.
bool carregar_fen(Tabuleiro& b, const std::string& fen);

// Gera a FEN da posição atual.
/*
P = Peão (Pawn)

N = Cavalo (Knight)

B = Bispo (Bishop)

R = Torre (Rook)

Q = Dama (Queen)

K = Rei (King)
*/

// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

/*
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR: As peças pretas na 8ª e 7ª fileiras, 4 fileiras vazias de 8 casas, e as peças brancas na 2ª e 1ª fileiras.

w: As Brancas jogam primeiro.

KQkq: Ambos os lados têm direito a ambos os roques.

-: Nenhuma captura en passant disponível.

0: 0 meias-jogadas sem captura ou avanço de peão.

1: Lance número 1.
*/
std::string gerar_fen(const Tabuleiro& b);