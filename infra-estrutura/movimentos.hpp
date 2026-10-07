#pragma once
#include <cstdint>
#include "lances.hpp"

// MEXER EM POUCA COISA, AQUI É TERRITÓRIO DA MÁQUINA DE REGRAS.

// Lista de lances de uma posição, com uma prioridade por lance.
// A prioridade NÃO é uma avaliação da posição: é só um palpite barato de "quão
// promissor é este lance", usado para decidir a ORDEM em que a busca os testa.
// (Os arrays não são zerados de propósito, por desempenho.)
struct ListaLances {
    Lance lances[MAX_LANCES];
    int   prioridades[MAX_LANCES];   // quanto maior, mais cedo o lance é testado
    int   quantidade = 0;

    void adicionar(Lance m) {
        // TODO: validar com assert(quantidade < MAX_LANCES)
        lances[quantidade] = m;
        prioridades[quantidade] = 0;   // calculada depois, em pontuar_lances
        quantidade++;
    }
};

// Gera todos os lances PSEUDO-LEGAIS: podem deixar o próprio rei em xeque.
// fazer_lance verifica isso e retorna false para lances ilegais.
void gerar_lances(const Tabuleiro& b, ListaLances& lista);

// Gera APENAS capturas e promoções (usada na quiescence search).
void gerar_capturas(const Tabuleiro& b, ListaLances& lista);

// A 'casa' está atacada por alguma peça de cor 'atacante'?
bool casa_atacada(const Tabuleiro& b, Quadrado casa, Cor atacante);

// O rei da 'cor' está em xeque?
bool em_xeque(const Tabuleiro& b, Cor cor);

// Ordenação de lances (move ordering). Quanto melhor a ordem, mais o alpha-beta corta.
// Ordem usual:
//   1) lance da tabela de transposição (lance_tt), se existir;
//   2) capturas, por MVV-LVA (vítima mais valiosa / atacante menos valioso),
//      depois refinadas por SEE (troca estática) se a engine evoluir;
//   3) demais lances (killers, histórico, etc., no futuro).
void pontuar_lances(const Tabuleiro& b, ListaLances& lista, Lance lance_tt);

// Ordenação incremental: coloca na posição 'indice_atual' o lance de maior prioridade
// entre os que restam (indice_atual .. quantidade-1).
void ordenar_proximo_lance(ListaLances& lista, int indice_atual);

// Conta os nós folha da árvore de lances legais até a profundidade dada.
// É a ferramenta padrão para validar a geração de lances contra números conhecidos.
uint64_t perft(Tabuleiro& b, int profundidade);