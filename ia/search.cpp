int minimax_alfa_beta(Tabuleiro *tabuleiro, int profundidade, int alfa, int beta, bool eh_maximizador){
    if(profundidade == 0 || jogoAcabou(tabuleiro)){
        return avaliar_posicao(tabuleiro);
    }
    else  if(eh_maximizador){
        int melhor_valor = INT_MIN;
        int *lances = lancesPossiveis(tabuleiro); //atualiza tabuleiro para novo lance
        for(int i=0;i<lances.length;i++){
            fazer_lance(tabuleiro, lances[i]);
            int valor = minimax(tabuleiro, profundidade-1, alfa, beta, false);
            desfazer_lance(tabuleiro, lances[i]);
            melhor_valor = std::max(melhor_valor, valor);
            alfa = std::max(alfa, melhor_valor);
            if(alfa >= beta) 
              break;
        }
        return melhor_valor;
    }
    else{
        int melhor_valor = INT_MAX;
        int *lances = lancesPossiveis(tabuleiro);
         for(int i=0;i<lances.length;i++){
            fazer_lance(tabuleiro, lances[i]);
            int valor = minimax(tabuleiro, profundidade-1, alfa, beta, true);
            desfazer_lance(tabuleiro, lances[i]);
            melhor_valor = std::min(melhor_valor, valor);
            beta = std::min(alfa, melhor_valor);
            if(alfa >= beta)
              break;
        }
        return melhor_valor;
    }
} 