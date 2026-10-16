#include "lances.hpp"
#include "tabuleiro/tabuleiro.hpp"
#include "utils/tipos.hpp"
#include <initializer_list>
#include "otimizacao/zobrist.hpp"
#include "movimentos.hpp"


static uint8_t novos_direitos(uint8_t r, Quadrado origem, Quadrado destino) {
    for (Quadrado q : {origem, destino}) {
        switch (q) {
            case e1: 
                r &= ~(ROQUE_BRANCO_REI | ROQUE_BRANCO_DAMA); 
                break;
            case h1: 
                r &= ~ROQUE_BRANCO_REI;  
                break;
            case a1: 
                r &= ~ROQUE_BRANCO_DAMA; 
                break;
            case e8: 
                r &= ~(ROQUE_PRETO_REI | ROQUE_PRETO_DAMA);   
                break;
            case h8: 
                r &= ~ROQUE_PRETO_REI;   
                break;
            case a8: 
                r &= ~ROQUE_PRETO_DAMA;  
                break;
            default: break;
        }
    }
    return r;
}


//Assume que a maquina de regras já validou todas as jogadas, logo podemos mover
//sem problemas.
//Por exemplo, do jeito que está estruturado uma rainha poderia fazer avanço duplo (en passant)
//Caso a maquina de regras falhe, aqui certamente irá falhar

bool fazer_lance(Tabuleiro& b, Lance m) {
    Quadrado origem = origem_do_lance(m), destino = destino_do_lance(m);
    Cor cor_atual = b.estado.lado_a_jogar, cor_oponente = oposta(cor_atual);
    TipoPeca tpeca = peca_em(b, origem).tpeca;

    uint64_t chave = b.estado.chave_zobrist;

    //historico é uma pilha que apontava para o vazio, preenche o vazio e incrementa o conta_lance
    b.historico[b.conta_lance++] = b.estado;
    //Estamos num novo meio-lance, limpamos informações antigas
    b.estado.capturada = TipoPeca::PECA_VAZIA;

    //Se for lance de peao ou captura, o contador de empate 50 volta pra zero
    if (tpeca == TipoPeca::PEAO || eh_captura(m)) b.estado.contador_50 = 0;
    else b.estado.contador_50++;    

    // O en passant antigo expira em qualquer lance (sai da chave, se existia)
    if (b.estado.en_passant != Quadrado::quadrado_vazio) chave ^= ZOBRIST[ZOB_EP + coluna_de(b.estado.en_passant)];
    b.estado.en_passant = Quadrado::quadrado_vazio;

    //Remove a peça capturada no destino, mas não toca na peça que fez a captura
    //isso será tratado no próximo if
    if (eh_captura(m)) {
        Quadrado cap;
        //Se for en passant, a peça está acima ou abaixo do quadrado
        if (eh_en_passant(m)) {
            cap = (cor_atual == Cor::BRANCO) ? static_cast<Quadrado>(destino-8):
                                               static_cast<Quadrado>(destino+8);
        }
        //A captura é no próprio destino
        else cap = destino;

        Peca p = peca_em(b, cap);
        //O tipo de peça que foi capturada, serve para depois desempilhar
        b.estado.capturada = p.tpeca;
    
        remover_peca(b, cor_oponente, b.estado.capturada, cap);
        chave ^= ZOBRIST[zob_peca(cor_oponente, p.tpeca, cap)];
    }

    if (eh_promocao(m)) {
        //Remove o peao e adiciona a peça selecionada
        remover_peca(b, cor_atual, TipoPeca::PEAO, origem);
        colocar_peca(b, cor_atual, tipo_da_promocao(m), destino);
        chave ^= ZOBRIST[zob_peca(cor_atual, TipoPeca::PEAO, origem)]
              ^  ZOBRIST[zob_peca(cor_atual, tipo_da_promocao(m), destino)];
    }
    else {
        //Peao está fazendo avanço duplo, temos casa de en passant
        if (eh_avanco_duplo(m)) {
            mover_peca(b, cor_atual, TipoPeca::PEAO, origem, destino);
            b.estado.en_passant = static_cast<Quadrado>((origem + destino) / 2);
            chave ^= ZOBRIST[ZOB_EP + coluna_de(b.estado.en_passant)];
        } 
        //Se não, movemos a peça normalmente
        else mover_peca(b, cor_atual, tpeca, origem, destino);

        chave ^= ZOBRIST[zob_peca(cor_atual, tpeca, origem)]
              ^  ZOBRIST[zob_peca(cor_atual, tpeca, destino)];
    }

    //Se tudo está certo, o rei já foi movido no "else" acima, falta a torre
    if (eh_roque(m)) {
        const bool branco   = (cor_atual == Cor::BRANCO);
        const bool lado_rei = (flags_do_lance(m) == LANCE_ROQUE_REI);
        const Quadrado t_origem  = lado_rei ? (branco ? h1 : h8) : (branco ? a1 : a8);
        const Quadrado t_destino = lado_rei ? (branco ? f1 : f8) : (branco ? d1 : d8);

        mover_peca(b, cor_atual, TipoPeca::TORRE, t_origem, t_destino);
        chave ^= ZOBRIST[zob_peca(cor_atual, TipoPeca::TORRE, t_origem)] 
              ^  ZOBRIST[zob_peca(cor_atual, TipoPeca::TORRE, t_destino)];
    }

    //Remove os direitos de roque anteriores
    chave ^= ZOBRIST[ZOB_ROQUE + b.estado.direitos_roque];

    //Adiciona os direitos novos
    b.estado.direitos_roque = novos_direitos(b.estado.direitos_roque, origem, destino);
    chave ^= ZOBRIST[ZOB_ROQUE + b.estado.direitos_roque];

    //Troca o lado a jogar
    b.estado.lado_a_jogar = cor_oponente;
    //Se for vez das brancas, desfaz a mudança, se for vez das pretas, aplica a hash
    //Lembrando da propriedade que A XOR B XOR B = A. (aplicado 2 vezes, é eliminado)
    chave ^= ZOBRIST[ZOB_LADO];

    b.estado.chave_zobrist = chave;

    //Se o rei ficou em xeque, desfaça o lance
    if (em_xeque(b, cor_atual)) {
        desfazer_lance(b, m);
        return false;
    }
    return true;
}

void desfazer_lance(Tabuleiro& b, Lance m) {
    //Lado a jogar foi atualizado, desatualize ele (troque de cor)

    Quadrado origem = origem_do_lance(m), destino = destino_do_lance(m);
    Cor cor_atual = oposta(b.estado.lado_a_jogar), cor_oponente = b.estao.lado_a_jogar;
    TipoPeca tpeca = peca_em(b, destino).tpeca;

    if (eh_promocao(m)) {
        //Remove o peao e adiciona a peça selecionada
        colocar_peca(b, cor_atual, TipoPeca::PEAO, origem);
        remover_peca(b, cor_atual, tipo_da_promocao(m), destino);
    }
    else {
        //Avanço duplo ou não, foi um movimento, só é necessário
        mover_peca(b, cor_atual, tpeca, destino, origem);
    }

    if (eh_captura(m)) {
        Quadrado cap;
        //Se for en passant, a peça está acima ou abaixo do quadrado
        if (eh_en_passant(m)) {
            cap = (cor_atual == Cor::BRANCO) ? static_cast<Quadrado>(destino-8):
                                               static_cast<Quadrado>(destino+8);
        }
        //A captura é no próprio destino
        else cap = destino;

        colocar_peca(b, cor_oponente, b.estado.capturada, cap);
    }

    if (eh_roque(m)) {
        Quadrado origem_mov, destino_mov;
        bool branco = (cor_atual == Cor::BRANCO);
        bool lado_rei = flags_do_lance(m) == LANCE_ROQUE_REI;
        origem_mov = lado_rei ? (branco ? f1 : f8) : (branco ? d1 : d8); 
        destino_mov = lado_rei ? (branco ? h1 : h8) : (branco ? a1 : a8);
        
        mover_peca(b, cor_atual, TipoPeca::TORRE, origem_mov, destino_mov);
    }

    b.estado = b.historico[--b.conta_lance];
}
