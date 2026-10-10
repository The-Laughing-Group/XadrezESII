#include "tabuleiro.hpp"
#include "bitboard.hpp"
#include <iostream>

void limpar(Tabuleiro& b) {
    b = Tabuleiro{};
}

void posicao_inicial(Tabuleiro& b) {
    limpar(b);

    TipoPeca ordem_xadrez[8] = {
        TipoPeca::TORRE, TipoPeca::CAVALO, TipoPeca::BISPO, TipoPeca::DAMA,
        TipoPeca::REI,   TipoPeca::BISPO,  TipoPeca::CAVALO, TipoPeca::TORRE
    };

    for (int i = 0; i < 8; i++) {
        Coluna col = static_cast<Coluna>(i);
        colocar_peca(b, Cor::BRANCO, ordem_xadrez[i], criar_quadrado(col, Linha::LINHA_1));
        colocar_peca(b, Cor::BRANCO, TipoPeca::PEAO, criar_quadrado(col, Linha::LINHA_2));
        colocar_peca(b, Cor::PRETO, TipoPeca::PEAO, criar_quadrado(col, Linha::LINHA_7));
        colocar_peca(b, Cor::PRETO, ordem_xadrez[i], criar_quadrado(col, Linha::LINHA_8));
    }

    //b.estado.chave_zobrist = calcular_chave(b); //WIP
}

void atualizar_ocupacao(Tabuleiro& b) {
    //
}

void colocar_peca(Tabuleiro& b, Cor cor, TipoPeca tpeca, Quadrado sq) {
    Peca p;
    p.cor = cor;
    p.tpeca = tpeca;
    b.casas[static_cast<int>(sq)] = p;

    BitboardUtils::setar_bit(b.pecas[idx(cor)][idx(tpeca)], sq);
    BitboardUtils::setar_bit(b.ocupacoes[idx(cor)], sq);
    BitboardUtils::setar_bit(b.ocupacoes_totais, sq);
}

void remover_peca(Tabuleiro& b, Cor cor, TipoPeca tpeca, Quadrado sq) {
    Peca p;
    p.cor = Cor::COR_VAZIA;
    p.tpeca = TipoPeca::PECA_VAZIA;
    b.casas[static_cast<int>(sq)] = p;

    BitboardUtils::limpar_bit(b.pecas[idx(cor)][idx(tpeca)], sq);
    BitboardUtils::limpar_bit(b.ocupacoes[idx(cor)], sq);
    BitboardUtils::limpar_bit(b.ocupacoes_totais, sq);
}

void mover_peca(Tabuleiro& b, Cor cor, TipoPeca tpeca, Quadrado origem, Quadrado destino) {
    Peca p;
    p.cor = Cor::COR_VAZIA;
    p.tpeca = TipoPeca::PECA_VAZIA;
    b.casas[static_cast<int>(origem)] = p;

    p.cor = cor;
    p.tpeca = tpeca;
    b.casas[static_cast<int>(destino)] = p;

    Bitboard orig = 0ULL, dest = 0ULL;
    orig = BitboardUtils::mascara_casa(origem);
    dest = BitboardUtils::mascara_casa(destino);
    b.pecas[idx(cor)][idx(tpeca)] ^= orig | dest;
    b.ocupacoes[idx(cor)] ^= orig | dest;
    b.ocupacoes_totais ^= orig | dest;
}

Peca peca_em(const Tabuleiro& b, Quadrado sq) {
    return b.casas[static_cast<int>(sq)];
}

bool casa_vazia(const Tabuleiro& b, Quadrado sq) {
    return BitboardUtils::testar_bit(b.ocupacoes_totais, sq);
}

int quantidade_de(const Tabuleiro& b, Cor cor, TipoPeca tpeca) {
    return BitboardUtils::contar_bits(b.pecas[idx(cor)][idx(tpeca)]);
}

Quadrado casa_do_rei(const Tabuleiro& b, Cor cor) {
    return BitboardUtils::indice_lsb(b.pecas[idx(cor)][idx(TipoPeca::REI)]);
}

bool verificar_consistencia(const Tabuleiro& b) {
    Peca p;
    for (int i = 0; i < 64; i++) {
        p = b.casas[i];
        if (p.tpeca == TipoPeca::PECA_VAZIA) {
            if (BitboardUtils::testar_bit(b.ocupacoes_totais, static_cast<Quadrado>(i))) return false;
            if (BitboardUtils::testar_bit(b.ocupacoes[idx(p.cor)], static_cast<Quadrado>(i))) return false;
        }
        else {
            if (!BitboardUtils::testar_bit(b.ocupacoes_totais, static_cast<Quadrado>(i))) return false;
            if (!BitboardUtils::testar_bit(b.ocupacoes[idx(p.cor)], static_cast<Quadrado>(i))) return false;
            if (!BitboardUtils::testar_bit(b.pecas[idx(p.cor)][idx(p.tpeca)], static_cast<Quadrado>(i))) return false;
        }
    }
}

void imprimir(const Tabuleiro& b) {
    char CARACTERE[NUM_CORES][NUM_TIPOS] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},   // brancas  (índice 0)
        {'p', 'n', 'b', 'r', 'q', 'k'}    // pretas   (índice 1)
    };
    Peca p;

    std::cout << "\n* A B C D E F G H\n";
    for (int i = 56; i >= 0; i = i - 8) {
        std::cout << (i / 8) + 1 << "|";
        for (int j = 0; j < 8; j++) {
            p = b.casas[i+j];
            std::cout << CARACTERE[idx(p.cor)][idx(p.tpeca)] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

bool carregar_fen(Tabuleiro& b, const std::string& fen) {
    //regras
}