#include "search.hpp"

int minimax_alfa_beta(Tabuleiro& tabuleiro, int profundidade, int alfa, int beta, bool eh_maximizador){
    //jogoAcabou ainda não foi definido
    if(profundidade == 0 || jogoAcabou(tabuleiro)){
        return avaliar(tabuleiro);
    }
    else  if(eh_maximizador){
        int melhor_valor = INT_MIN;
        ListaLances lista;
        gerar_lances(tabuleiro, lista); // Preenche a lista de lances pseudo-legais
        for(int i=0;i<lista.quantidade;i++){
            Lance lance = lista.lances[i];
            if(!fazer_lance(tabuleiro, lance)) continue;

            int valor = minimax_alfa_beta(tabuleiro, profundidade-1, alfa, beta, false);

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

            int valor = minimax_alfa_beta(tabuleiro, profundidade-1, alfa, beta, true);
            
            desfazer_lance(tabuleiro, lance);
            melhor_valor = std::min(melhor_valor, valor);

            beta = std::min(beta, melhor_valor);
            if(alfa >= beta)
              break;
        }
        return melhor_valor;
    }
} 