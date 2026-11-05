#include "zobrist.hpp"
#include "../bitboard/bitboard.hpp"
#include <random>

void iniciar_zobrist() {
    std::mt19937_64 gerador(0x1234567ULL);   // semente fixa: mesma sequência em toda execução
    for (int i = 0; i < ZOB_TOTAL; i++)
        ZOBRIST[i] = gerador();              // cada chamada devolve um uint64_t aleatório
}

//Usado para testes, ver se o calculo da chave de forma incremental está certo (no fazer_lance)
//Além disso, usada em posições iniciais
uint64_t calcular_chave(const Tabuleiro& b) {
    uint64_t chave = 0ULL;
    Quadrado sq = Quadrado::quadrado_vazio;

    
    //Calcula a chave de cada peça em cada quadrado
    for (int c = 0; c < NUM_CORES; c++) {
        for (int tp = 0; tp < NUM_TIPOS; tp++) {
            Bitboard pecas_copia = b.pecas[c][tp];
            while (pecas_copia) {
                sq = BitboardUtils::remover_lsb(pecas_copia);
                chave ^= ZOBRIST[zob_peca(static_cast<Cor>(c), static_cast<TipoPeca>(tp), sq)];
            }
        }
    }

    //Adiciona na chave as flags
    chave ^= ZOBRIST[ZOB_ROQUE + b.estado.direitos_roque];
    if (b.estado.en_passant != Quadrado::quadrado_vazio) {
        chave ^= ZOBRIST[ZOB_EP + coluna_de(b.estado.en_passant)];
    }
    if (b.estado.lado_a_jogar == Cor::PRETO) {
        chave ^= ZOBRIST[ZOB_LADO];
    }

    return chave;
}
