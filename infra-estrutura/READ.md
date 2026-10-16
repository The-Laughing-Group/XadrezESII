# Nome do projeto

Engine de xadrez em C++ (bitboards + piece-list + mailbox). O uso dessas estruturas visa otimizar a busca.

## Requisitos
- Compilador com suporte a **C++20** (g++ 10+ ou clang 11+)

## Como compilar
    g++ -std=c++20 -O2 -Wall -Wextra *.cpp -o engine

## Organização dos arquivos
| Arquivo | O que contém |
|---|---|
| tipos.hpp | enums, máscaras, `Estado`, `Tabuleiro` |
| bitboard.hpp | operações de bits |
| lances.hpp | formato do `Lance` (16 bits), fazer/desfazer lance |
| movimentos.hpp | geração e ordenação de lances (máquina de regras) |
| tabuleiro.hpp | colocar/remover peças, consultas, FEN |
| avaliacao.hpp | função de avaliação |
| tt.hpp | tabela de transposição |
| zobrist.hpp | chaves de hash da posição |

## Convenções (leia antes de mexer)
- Casas em LERF: a1 = 0, b1 = 1, ..., h8 = 63.
- `Lance`: bits 0-5 origem, 6-11 destino, 12-15 flags (tabela de flags em lances.hpp).
- Roque: origem/destino são os do **rei**.
- En passant: o peão capturado fica em `destino ∓ 8`, não no destino.
- Só `colocar_peca`, `remover_peca` e `mover_peca` alteram as peças do tabuleiro.
- Nunca passar `Tabuleiro` por valor (tem ~48 KB).

## Ordem de inicialização
`iniciar_zobrist()` antes de `posicao_inicial()`.

## Testes


## Estado atual
Os hpp estão prontos, falta a implementação em cpp.