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

    tabela_tt.resize(entradas_arredondadas);

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
void nova_busca_tt() {
    idade_atual++;
}

bool ler_tt(uint64_t chave, int profundidade, int ply, int alpha, int beta, int& pontuacao, Lance& melhor_lance) {
    const TTEntry& entrada_tt = tabela_tt[chave & tt_mascara];

    melhor_lance = SEM_LANCE;

    if (entrada_tt.chave == chave) {

        melhor_lance = entrada_tt.melhor_lance;

        if (profundidade <= entrada_tt.profundidade) {

            const int pontuacao_tt = pontuacao_da_tt(entrada_tt.pontuacao, ply);
            // Valor Exato, da pra cortar pois é o melhor lance
            if (entrada_tt.flag == TTFlag::EXACT) {
                pontuacao = pontuacao_tt;
                return true;
            }
            // Corte
            // LOWERBOUND -> o valor da posição eh pelo menos pontuacao_tt
            // Se pontuacao_tt for maior que beta, ocorre corte
            if (entrada_tt.flag == TTFlag::LOWERBOUND && pontuacao_tt >= beta) {
                pontuacao = pontuacao_tt;
                return true;
            }
            // Corte
            // UPPERBOUND -> o valor da posição eh no máximo pontuacao_tt
            // Se pontuacao_tt for menor que alpha, ocorre corte
            if (entrada_tt.flag == TTFlag::UPPERBOUND && pontuacao_tt <= alpha) {
                pontuacao = pontuacao_tt;
                return true;
            }
        }
    }
    return false;
}

void gravar_tt(uint64_t chave, int profundidade, int ply, int pontuacao, TTFlag flag, Lance melhor_lance) {
    TTEntry& entrada_tt = tabela_tt[chave & tt_mascara];

    // Decidimos se vamos sobrescrever a entrada atual da TT.
    // Substituimos se:
    // 1. A chave for diferente (posição nova ou slot livre).
    // 2. A entrada antiga for de uma geração/turno passado (idade diferente).
    // 3. A nova busca tiver profundidade maior ou igual à armazenada.
    bool deve_substituir = (entrada_tt.chave != chave) ||
                           (entrada_tt.idade != idade_atual) ||
                           (profundidade >= entrada_tt.profundidade);

    if (deve_substituir) {
        // Proteção extra: se for exatamente a mesma posição e a nova busca 
        // não encontrou nenhum lance válido (SEM_LANCE), mas a antiga tinha,
        // preservamos o melhor lance antigo para não perder a ordenação.
        if (entrada_tt.chave == chave && melhor_lance == SEM_LANCE) {
            melhor_lance = entrada_tt.melhor_lance;
        }

        entrada_tt.chave = chave;
        entrada_tt.flag = flag;
        entrada_tt.melhor_lance = melhor_lance;
        entrada_tt.pontuacao = pontuacao_para_tt(pontuacao, ply);
        entrada_tt.profundidade = profundidade;
        entrada_tt.idade = idade_atual;
    }
}