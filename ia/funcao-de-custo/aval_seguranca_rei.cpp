#include "aval_seguranca_rei.hpp"

#include <bit>

namespace {

constexpr int PENALIDADE_PEAO_AUSENTE_ESCUDO = 10;
constexpr int PENALIDADE_COLUNA_ABERTA_REI = 18;
constexpr int PENALIDADE_COLUNA_ABERTA_ADJACENTE = 9;

constexpr int coluna_da_casa(int casa) {
    return casa % NUM_COLUNAS;
}

constexpr int linha_da_casa(int casa) {
    return casa / NUM_COLUNAS;
}

int avaliar_escudo_peoes(int casa_rei, Bitboard peoes, Cor cor) {
    const int coluna_rei = coluna_da_casa(casa_rei);
    const int linha_rei = linha_da_casa(casa_rei);
    const int direcao = cor == Cor::BRANCO ? 1 : -1;
    const int linha_escudo = linha_rei + direcao;

    if (linha_escudo < 0 || linha_escudo >= NUM_LINHAS) {
        return 0;
    }

    int penalidade = 0;
    for (int deslocamento = -1; deslocamento <= 1; ++deslocamento) {
        const int coluna = coluna_rei + deslocamento;
        if (coluna < 0 || coluna >= NUM_COLUNAS) {
            continue;
        }

        const int casa = linha_escudo * NUM_COLUNAS + coluna;
        const Bitboard mascara = Bitboard{1} << casa;
        if ((peoes & mascara) == 0) {
            penalidade += PENALIDADE_PEAO_AUSENTE_ESCUDO;
        }
    }

    return penalidade;
}

bool ha_peca_pesada_na_coluna(Bitboard pecas_pesadas, int coluna) {
    return (pecas_pesadas & MASCARA_COLUNAS[coluna]) != 0;
}

int avaliar_colunas_abertas(const Tabuleiro& tab, int casa_rei, Cor cor) {
    const int coluna_rei = coluna_da_casa(casa_rei);
    const Cor cor_adversaria = oposta(cor);

    const Bitboard todos_os_peoes =
        tab.pecas[idx(Cor::BRANCO)][idx(TipoPeca::PEAO)] |
        tab.pecas[idx(Cor::PRETO)][idx(TipoPeca::PEAO)];
    const Bitboard pecas_pesadas_adversarias =
        tab.pecas[idx(cor_adversaria)][idx(TipoPeca::TORRE)] |
        tab.pecas[idx(cor_adversaria)][idx(TipoPeca::DAMA)];

    int penalidade = 0;

    // Considera a coluna do rei e as duas adjacentes, respeitando as bordas.
    for (int deslocamento = -1; deslocamento <= 1; ++deslocamento) {
        const int coluna = coluna_rei + deslocamento;
        if (coluna < 0 || coluna >= NUM_COLUNAS) {
            continue;
        }

        const Bitboard mascara_coluna = MASCARA_COLUNAS[coluna];
        const bool coluna_aberta = (todos_os_peoes & mascara_coluna) == 0;

        // Uma coluna vazia de peões só representa perigo quando o adversário
        // tem torre ou dama nessa coluna.
        if (coluna_aberta &&
            ha_peca_pesada_na_coluna(pecas_pesadas_adversarias, coluna)) {
            penalidade += deslocamento == 0
                ? PENALIDADE_COLUNA_ABERTA_REI
                : PENALIDADE_COLUNA_ABERTA_ADJACENTE;
        }
    }

    return penalidade;
}

} // namespace

int avaliar_seguranca_rei(const Tabuleiro& tab) {
    int score = 0;

    for (int indice_cor = 0; indice_cor < NUM_CORES; ++indice_cor) {
        const Cor cor = static_cast<Cor>(indice_cor);
        const int sinal = cor == Cor::BRANCO ? 1 : -1;
        const Bitboard reis = tab.pecas[idx(cor)][idx(TipoPeca::REI)];

        // Posições inválidas sem rei não devem quebrar a avaliação.
        if (reis == 0) {
            continue;
        }

        const int casa_rei = std::countr_zero(reis);
        const Bitboard peoes = tab.pecas[idx(cor)][idx(TipoPeca::PEAO)];

        const int penalidade_escudo =
            avaliar_escudo_peoes(casa_rei, peoes, cor);
        const int penalidade_colunas =
            avaliar_colunas_abertas(tab, casa_rei, cor);

        score -= sinal * (penalidade_escudo + penalidade_colunas);
    }

    return score;
}
