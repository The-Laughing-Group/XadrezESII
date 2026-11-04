#include "tt.hpp"
#include <bit>
#include <vector>
#include <algorithm>

std::vector<TTEntry> tabela_tt;
size_t tt_mascara = 0;
uint8_t idade_atual = 0;

void iniciar_tt(std::size_t megabytes) {
    size_t total_bytes = megabytes * 1024 * 1024;
    size_t num_entradas = total_bytes / sizeof(TTEntry);

    //Arredonda
    size_t entradas_arredondadas = std::bit_floor(num_entradas);

    tabela_tt = std::vector<TTEntry>(entradas_arredondadas);

    //Mascara para a chave zobrist não acessar um indice não existente
    tt_mascara = entradas_arredondadas - 1;
}

void limpar_tt() {
    // Limpa tudo com zero
    std::fill(tabela_tt.begin(), tabela_tt.end(), TTEntry{});
}

/*
O fluxo do calculo de idade:
    Início do turno da engine: A engine vai começar a calcular o lance que ela vai jogar agora. É aqui que você chama 
    nova_busca_tt() uma única vez. A idade global da TT sobe (ex: de 0 para 1).
    
    O iterative deepening roda: A engine faz a busca completa para este turno usando o 
    iterative deepening ($d=1, d=2, d=3, \dots$). Durante todo este processo de busca, a idade permanece fixa em 1. 
    
    Todas as profundidades compartilham e atualizam as entradas livremente sem envelhecer umas às outras.
    A engine joga: O tempo acaba, a engine escolhe o melhor lance e o executa no tabuleiro.
    
    Turno do adversário: O adversário responde.
    
    Novo turno da engine: A engine vai pensar de novo. Chamar nova_busca_tt() de novo (idade vai para 2), e o ciclo se repete.
*/
void nova_busca() {
    idade_atual++;
}

bool ler_tt(uint64_t chave, int profundidade, int ply, int alpha, int beta, int& pontuacao, Lance& melhor_lance) {
    
}