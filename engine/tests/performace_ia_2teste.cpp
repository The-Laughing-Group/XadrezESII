// performace_ia_teste.cpp
// Posição-base FIXA (rei/torres/peões nos cantos), SÓ as fileiras 4 e 5
// (linhas 3 e 4 do array) são preenchidas aleatoriamente. Mede o tempo da IA.
// Compile junto com a engine xadrez.

#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "xadrez/jogo.hpp"

using namespace xadrez;

// ============================================================
// CONFIGURAÇÃO — mexa só aqui
// ============================================================
constexpr int       MINUTOS_DE_TESTE     = 1;    // quanto tempo o teste roda
constexpr long long LIMITE_MS_POR_JOGADA = 9000; // tempo máximo por lance da IA
constexpr int       PROFUNDIDADE_IA      = 5;    // profundidade da busca
constexpr unsigned  SEMENTE_RNG          = 66;   // semente fixa: reproduzível
// ============================================================

// ------------------------------------------------------------
// Base fixa (igual à FEN 6rk/5ppp/6p1/8/8/1P6/PPP5/KR6).
// Só as linhas 3 e 4 (5ª e 4ª fileiras) ficam livres para sorteio.
// ------------------------------------------------------------
static void montarBaseFixa(char tab[8][8]) {
    for (int l = 0; l < 8; ++l)
        for (int c = 0; c < 8; ++c)
            tab[l][c] = '.';

    // Linha 0 (8ª fileira): "6rk"
    tab[0][6] = 'r';
    tab[0][7] = 'k';

    // Linha 1 (7ª fileira): "5ppp"
    tab[1][5] = 'p';
    tab[1][6] = 'p';
    tab[1][7] = 'p';

    // Linha 2 (6ª fileira): "6p1"
    tab[2][6] = 'p';

    // Linha 3 (5ª fileira): "8"   -> livre para sorteio
    // Linha 4 (4ª fileira): "8"   -> livre para sorteio

    // Linha 5 (3ª fileira): "1P6"
    tab[5][1] = 'P';

    // Linha 6 (2ª fileira): "PPP5"
    tab[6][0] = 'P';
    tab[6][1] = 'P';
    tab[6][2] = 'P';

    // Linha 7 (1ª fileira): "KR6"
    tab[7][0] = 'K';
    tab[7][1] = 'R';
}

// ------------------------------------------------------------
// Sorteia uma FEN:
//   - base FIXA acima;
//   - APENAS as linhas 3 e 4 (16 casas) recebem peças aleatórias;
//   - sem reis no sorteio (os reis são só os da base);
//   - vez das PRETAS, sem roque, sem en passant.
// Como as linhas 3 e 4 não são a 1ª nem a 8ª fileira, peões sorteados
// nelas são sempre válidos -> não precisa de critério de peão.
// ------------------------------------------------------------
static std::string sortearFen(std::mt19937& rng) {
    // Símbolos possíveis (sem reis).
    const char SIMBOLOS[] = {
        'P','N','B','R','Q',   // brancas
        'p','n','b','r','q'    // pretas
    };
    const int NUM_SIMBOLOS = 10;

    char tab[8][8];
    montarBaseFixa(tab);

    // Cada uma das 16 casas (linhas 3 e 4) recebe uma peça sorteada
    // OU fica vazia. Vazio também é uma opção válida, então sorteamos
    // entre 11 possibilidades: 10 peças + "vazio".
    std::uniform_int_distribution<int> escolha(0, NUM_SIMBOLOS); // 0..10
    for (int l = 3; l <= 4; ++l)
        for (int c = 0; c < 8; ++c) {
            const int r = escolha(rng);
            tab[l][c] = (r == NUM_SIMBOLOS) ? '.' : SIMBOLOS[r];
        }

    // Monta a FEN.
    std::ostringstream pecas;
    for (int l = 0; l < 8; ++l) {
        int vazias = 0;
        for (int c = 0; c < 8; ++c) {
            if (tab[l][c] == '.') {
                ++vazias;
            } else {
                if (vazias) { pecas << vazias; vazias = 0; }
                pecas << tab[l][c];
            }
        }
        if (vazias) pecas << vazias;
        if (l < 7) pecas << '/';
    }

    return pecas.str() + " b - - 0 1";
}

// ------------------------------------------------------------
// Teste
// ------------------------------------------------------------
TEST(IA_Performance, BaseFixaFileirasAleatoriasTempoPorJogada) {
    using namespace std::chrono;

    const auto inicio = steady_clock::now();
    const auto fim    = inicio + minutes(MINUTOS_DE_TESTE);

    std::mt19937 rng(SEMENTE_RNG);

    int iteracoes    = 0;  // posições válidas em que a IA foi medida
    int puladas_fen  = 0;  // FEN rejeitada por carregarFen
    int puladas_mate = 0;  // posição já terminada (mate/afogamento)

    while (steady_clock::now() < fim) {
        // Sorteia até achar FEN válida E não terminal.
        std::string fen;
        bool ok = false;
        for (int tent = 0; tent < 200 && !ok; ++tent) {
            const std::string candidata = sortearFen(rng);

            Jogo tmp;
            if (!tmp.carregarFen(candidata)) { ++puladas_fen;  continue; }
            if (tmp.situacao().terminou)     { ++puladas_mate; continue; }

            fen = candidata;
            ok  = true;
        }
        if (!ok) continue;

        Jogo j;
        ASSERT_TRUE(j.carregarFen(fen)) << "FEN sorteada e rejeitada: " << fen;

        ASSERT_FALSE(j.vezDasBrancas())
            << "FEN sorteada deveria ter vez das pretas: " << fen;

        // ---- Mede o tempo da IA ----
        const auto t0 = steady_clock::now();
        const std::string m = j.melhorLanceIA(PROFUNDIDADE_IA);
        const auto t1 = steady_clock::now();
        const long long ms = duration_cast<milliseconds>(t1 - t0).count();

        // ---- Verificações ----
        if (m.empty()) {
            const SituacaoPartida s = j.situacao();
            ASSERT_TRUE(s.terminou)
                << "IA devolveu vazio mas a partida NAO terminou\n"
                << "FEN: " << fen << "\nestado=" << s.estado;
            ++puladas_mate;
            continue;
        }

        ASSERT_TRUE(j.tentarLance(m))
            << "IA devolveu lance invalido: " << m << "\nFEN: " << fen;

        ASSERT_LT(ms, LIMITE_MS_POR_JOGADA)
            << "IA levou " << ms << " ms (limite " << LIMITE_MS_POR_JOGADA << ")"
            << "\nFEN: " << fen
            << "\nLance: " << m;

        ++iteracoes;
    }

    std::cout << "[IA_Performance] iteracoes=" << iteracoes
              << " puladas_fen=" << puladas_fen
              << " puladas_mate=" << puladas_mate
              << " minutos=" << MINUTOS_DE_TESTE
              << " profundidade=" << PROFUNDIDADE_IA << "\n";
}