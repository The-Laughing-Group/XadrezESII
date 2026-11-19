#include "search.hpp"
#include <limits.h>
#include <algorithm>
#include "../../infra-estrutura/otimizacao/tt.hpp"

bool g_debug_busca = false; 
long long g_nos = 0;
std::ostringstream g_log_busca;

static std::string lance_para_texto(Lance m) {
    auto sq = [](int s) { return std::string{char('a' + s % 8), char('1' + s / 8)}; };
    return sq(origem_do_lance(m)) + sq(destino_do_lance(m));
}

namespace {
    constexpr int PRIORIDADE_LANCE_TT = 1000000;

    // Pontuação sempre do ponto de vista das brancas.
    inline int pontuacao_de_mate(const Tabuleiro& t, int ply) {
        return t.estado.lado_a_jogar == Cor::BRANCO ? -SCORE_MATE + ply
                                                    :  SCORE_MATE - ply;
    }
}

int minimax_alfa_beta(Tabuleiro& tabuleiro, int profundidade, int alfa, int beta,
                      bool eh_maximizador, int ply)
{
    g_nos ++;
    const Cor lado = tabuleiro.estado.lado_a_jogar;

    // Empates por regra
    if (repetida(tabuleiro) || empate_50(tabuleiro)) return 0;

    // Folha: só confirma mate se estiver em xeque (evita gerar lances em toda folha)
    if (profundidade == 0) {
        if (em_xeque(tabuleiro, lado) && verificarFim(tabuleiro) == FimDeJogo::mate)
            return pontuacao_de_mate(tabuleiro, ply);
        return avaliar(tabuleiro);
    }

    // Tabela de transposição
    int pontuacao_tt;
    Lance lance_tt = SEM_LANCE;
    if (ler_tt(tabuleiro.estado.chave_zobrist, profundidade, ply, alfa, beta,
               pontuacao_tt, lance_tt))
        return pontuacao_tt;

    ListaLances lista;
    gerar_lances(tabuleiro, lista);

    // Prioridade máxima para o lance da TT (as demais prioridades vêm da geração)
    if (lance_tt != SEM_LANCE) {
        for (int i = 0; i < lista.quantidade; i++) {
            if (lista.lances[i] == lance_tt) {
                lista.prioridades[i] = PRIORIDADE_LANCE_TT;
                break;
            }
        }
    }

    const int alfa_original = alfa;
    const int beta_original = beta;
    int  melhor_valor = eh_maximizador ? -SCORE_INFINITO : SCORE_INFINITO;
    Lance melhor_lance = SEM_LANCE;
    int  legais = 0;

    for (int i = 0; i < lista.quantidade; i++) {
        // Seleção parcial: traz o lance de maior prioridade para a posição i
        int idx = i;
        for (int j = i + 1; j < lista.quantidade; j++)
            if (lista.prioridades[j] > lista.prioridades[idx]) idx = j;
        std::swap(lista.lances[i], lista.lances[idx]);
        std::swap(lista.prioridades[i], lista.prioridades[idx]);

        const Lance lance = lista.lances[i];
        if (!fazer_lance(tabuleiro, lance)) continue;
        legais++;

        const int valor = minimax_alfa_beta(tabuleiro, profundidade - 1, alfa, beta,
                                            !eh_maximizador, ply + 1);
        desfazer_lance(tabuleiro, lance);

        if (eh_maximizador ? valor > melhor_valor : valor < melhor_valor) {
            melhor_valor = valor;
            melhor_lance = lance;
        }

        if (eh_maximizador) alfa = std::max(alfa, melhor_valor);
        else                beta = std::min(beta, melhor_valor);

        if (alfa >= beta) break;
    }

    // Nenhum lance legal: mate ou afogamento (não grava na TT)
    if (legais == 0)
        return em_xeque(tabuleiro, lado) ? pontuacao_de_mate(tabuleiro, ply) : 0;

    // Classificação da entrada (vale para os dois lados, pois a pontuação é das brancas)
    TTFlag flag;
    if      (melhor_valor <= alfa_original) flag = TTFlag::UPPERBOUND;
    else if (melhor_valor >= beta_original) flag = TTFlag::LOWERBOUND;
    else                                    flag = TTFlag::EXACT;

    gravar_tt(tabuleiro.estado.chave_zobrist, profundidade, ply, melhor_valor, flag, melhor_lance);
    return melhor_valor;
}

Lance obter_melhor_lance(Tabuleiro& tabuleiro, int profundidade) {
    nova_busca_tt();   // uma vez por lance da engine
    g_nos = 0;

    const bool brancas = tabuleiro.estado.lado_a_jogar == Cor::BRANCO;

    ListaLances lista;
    gerar_lances(tabuleiro, lista);

    Lance melhor = SEM_LANCE;
    int melhor_valor = brancas ? -SCORE_INFINITO : SCORE_INFINITO;
    int alfa = -SCORE_INFINITO, beta = SCORE_INFINITO;

    for (int i = 0; i < lista.quantidade; ++i) {
        const Lance lance = lista.lances[i];
        if (!fazer_lance(tabuleiro, lance)) continue;

        const int valor = minimax_alfa_beta(tabuleiro, profundidade - 1, alfa, beta,
                                            !brancas, 1);   // ply 1 no filho da raiz
        desfazer_lance(tabuleiro, lance);

        if (g_debug_busca) g_log_busca << lance_para_texto(lance) << " -> " << valor << "\n";

        if (brancas ? valor > melhor_valor : valor < melhor_valor) {
            melhor_valor = valor;
            melhor = lance;
            if (brancas) alfa = std::max(alfa, melhor_valor);
            else         beta = std::min(beta, melhor_valor);
        }
    }

    if (g_debug_busca) g_log_busca << "melhor: " << lance_para_texto(melhor) << "  nos: " << g_nos << "\n";
    return melhor;
}