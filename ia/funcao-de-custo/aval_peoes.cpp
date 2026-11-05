#include "aval_peoes.hpp"

#include <bit>
#include <cstdlib>

namespace {

constexpr int PENALIDADE_PEAO_DOBRADO = 15;
constexpr int PENALIDADE_PEAO_ISOLADO = 12;
constexpr int BONUS_PEAO_PASSADO_BASE = 10;
constexpr int BONUS_PEAO_PASSADO_POR_LINHA = 8;
constexpr int MAX_AVANCO_PEAO = 5;

constexpr int coluna_da_casa(int casa) {
    return casa % NUM_COLUNAS;
}

constexpr int linha_da_casa(int casa) {
    return casa / NUM_COLUNAS;
}

bool tem_peao_aliado_adjacente(int casa, Bitboard peoes) {
    const int coluna = coluna_da_casa(casa);
    Bitboard colunas_adjacentes = 0;

    if (coluna > 0) {
        colunas_adjacentes |= MASCARA_COLUNAS[coluna - 1];
    }
    if (coluna < NUM_COLUNAS - 1) {
        colunas_adjacentes |= MASCARA_COLUNAS[coluna + 1];
    }

    return (peoes & colunas_adjacentes) != 0;
}

bool eh_peao_passado(int casa, Bitboard peoes_adversarios, Cor cor) {
    const int coluna = coluna_da_casa(casa);
    const int linha = linha_da_casa(casa);
    const bool brancas = cor == Cor::BRANCO;

    while (peoes_adversarios != 0) {
        const int casa_inimiga = std::countr_zero(peoes_adversarios);
        const int coluna_inimiga = coluna_da_casa(casa_inimiga);
        const int linha_inimiga = linha_da_casa(casa_inimiga);

        const bool mesma_ou_adjacente =
            std::abs(coluna - coluna_inimiga) <= 1;
        const bool a_frente = brancas
            ? linha_inimiga > linha
            : linha_inimiga < linha;

        if (mesma_ou_adjacente && a_frente) {
            return false;
        }

        peoes_adversarios &= peoes_adversarios - 1;
    }

    return true;
}

int bonus_peao_passado(int casa, Cor cor) {
    const int linha = linha_da_casa(casa);
    const int avanco = cor == Cor::BRANCO
        ? linha - Linha::LINHA_2
        : Linha::LINHA_7 - linha;

    // Considera apenas as seis fileiras entre a posição inicial e a promoção.
    const int avanco_limitado = avanco < 0
        ? 0
        : (avanco > MAX_AVANCO_PEAO ? MAX_AVANCO_PEAO : avanco);

    return BONUS_PEAO_PASSADO_BASE +
        avanco_limitado * BONUS_PEAO_PASSADO_POR_LINHA;
}

} // namespace

int avaliar_estrutura_peoes(const Tabuleiro& tab) {
    int avaliacao = 0;

    for (int indice_cor = 0; indice_cor < NUM_CORES; ++indice_cor) {
        const Cor cor = static_cast<Cor>(indice_cor);
        const Cor cor_adversaria = oposta(cor);
        const int sinal = cor == Cor::BRANCO ? 1 : -1;

        const Bitboard peoes =
            tab.pecas[idx(cor)][idx(TipoPeca::PEAO)];
        const Bitboard peoes_adversarios =
            tab.pecas[idx(cor_adversaria)][idx(TipoPeca::PEAO)];

        // Penaliza cada peão além do primeiro em uma mesma coluna.
        for (int indice_coluna = 0; indice_coluna < NUM_COLUNAS; ++indice_coluna) {
            const auto coluna = static_cast<Coluna>(indice_coluna);
            const int quantidade = std::popcount(
                peoes & get_mascara_coluna(coluna));

            if (quantidade > 1) {
                avaliacao -= sinal * PENALIDADE_PEAO_DOBRADO * (quantidade - 1);
            }
        }

        Bitboard restantes = peoes;
        while (restantes != 0) {
            const int casa = std::countr_zero(restantes);

            if (!tem_peao_aliado_adjacente(casa, peoes)) {
                avaliacao -= sinal * PENALIDADE_PEAO_ISOLADO;
            }

            if (eh_peao_passado(casa, peoes_adversarios, cor)) {
                avaliacao += sinal * bonus_peao_passado(casa, cor);
            }

            restantes &= restantes - 1;
        }
    }

    return avaliacao;
}
