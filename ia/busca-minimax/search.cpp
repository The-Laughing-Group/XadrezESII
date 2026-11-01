#include "search.hpp"
#include <limits.h>
#include <algorithm>

// Passar eh_maximizador como false na chamada da função caso a IA seja as peças pretas, 
// caso contrário passar true
// a variável ply conta em quantas jogadas cheguei no fim do jogo, é interessante para a minimax
// preferir um xeque-mate mais cedo, por exemplo
int minimax_alfa_beta(Tabuleiro& tabuleiro, int profundidade, int alfa, int beta, bool eh_maximizador, int ply){
    // Isso é necessário para conseguir calcular na avaliação se foi um xeque-mate ou um empate
    // o que muda a pontuação final daquela jogada
    const FimDeJogo fim = verificarFim(tabuleiro);
    if(fim == FimDeJogo::empate) return 0;
    // Lado branco na vez e mateado: vitória preta, score negativo.
    // Lado preto na vez e mateado: vitória branca, score positivo.
    else if(fim == FimDeJogo::mate){
        return  tabuleiro.estado.lado_a_jogar == Cor::BRANCO ? -SCORE_MATE + ply : SCORE_MATE - ply;
    }
    else if (profundidade == 0){
        return avaliar(tabuleiro);  
    }    
    else  if(eh_maximizador){
        int melhor_valor = INT_MIN;
        ListaLances lista;
        gerar_lances(tabuleiro, lista); // Preenche a lista de lances pseudo-legais
        for(int i=0;i<lista.quantidade;i++){
            Lance lance = lista.lances[i];
            if(!fazer_lance(tabuleiro, lance)) continue;

            int valor = minimax_alfa_beta(tabuleiro, profundidade-1, alfa, beta, false, ply + 1);

            desfazer_lance(tabuleiro, lance);
            melhor_valor = std::max(melhor_valor, valor);

            alfa = std::max(alfa, melhor_valor);
            if(alfa >= beta) 
              break;
        }
        return melhor_valor;
    }
    else{
        int melhor_valor = INT_MAX;
        ListaLances lista;
        gerar_lances(tabuleiro, lista);
         for(int i=0;i<lista.quantidade;i++){
            Lance lance = lista.lances[i];
            if(!fazer_lance(tabuleiro, lance)) continue;

            int valor = minimax_alfa_beta(tabuleiro, profundidade-1, alfa, beta, true, ply + 1);
            
            desfazer_lance(tabuleiro, lance);
            melhor_valor = std::min(melhor_valor, valor);

            beta = std::min(beta, melhor_valor);
            if(alfa >= beta)
              break;
        }
        return melhor_valor;
    }
}

Lance obter_melhor_lance(Tabuleiro& tabuleiro, int profundidade) {
    ListaLances lista;
    gerar_lances(tabuleiro, lista);

    Lance melhor = SEM_LANCE;
    int melhor_valor = INT_MAX; // pretas minimizam uma avaliação positiva para as brancas

    for (int i = 0; i < lista.quantidade; ++i) {
        Lance lance = lista.lances[i];

        if (!fazer_lance(tabuleiro, lance)) continue;

        int valor = minimax_alfa_beta(
            tabuleiro,
            profundidade - 1,
            INT_MIN,
            INT_MAX,
            true, // depois do lance preto, as brancas maximizam
            0
        );

        desfazer_lance(tabuleiro, lance);

        if (valor < melhor_valor) {
            melhor_valor = valor;
            melhor = lance;
        }
    }

    return melhor;
}