#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace xadrez;

constexpr long long LIMITE_MS = 5000;
constexpr int PROFUNDIDADE = 5;
constexpr int MAX_JOGADAS = 100;

// Sorteia um lance legal do lado que tem a vez.
static std::string lanceAleatorio(Jogo& j, std::mt19937& rng) {
    std::vector<std::string> lances;
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r)
            for (const auto& m : j.lancesDe(std::string{c, r}))
                lances.push_back(m);
    if (lances.empty()) return "";
    std::uniform_int_distribution<std::size_t> dist(0, lances.size() - 1);
    return lances[dist(rng)];
}

TEST(IA_Performance, PartidaCompletaTempoPorLance) {
    Jogo j;
    std::mt19937 rng(42);   // semente fixa: partida reproduzível. Troque para variar.
    int lance = 0;

    while (lance < MAX_JOGADAS) {
        if (j.situacao().terminou) break;

        const std::string fenAntes = j.fen();

        if (j.vezDasBrancas()) {
            const std::string m = lanceAleatorio(j, rng);
            if (m.empty()) break;
            ASSERT_TRUE(j.tentarLance(m))
                << "Lance das brancas invalido: " << m
                << "\nFEN antes: " << fenAntes;
        } else {
            const auto inicio = std::chrono::steady_clock::now();
            const std::string m = j.melhorLanceIA(PROFUNDIDADE);
            const auto fim = std::chrono::steady_clock::now();
            const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(fim - inicio).count();

            ASSERT_FALSE(m.empty())
                << "IA nao devolveu lance no lance " << lance
                << "\nFEN antes: " << fenAntes;

            ASSERT_LT(ms, LIMITE_MS)
                << "Lance " << lance << " da IA levou " << ms << " ms (limite " << LIMITE_MS << ")"
                << "\nLance escolhido: " << m
                << "\nFEN antes: " << fenAntes;

            ASSERT_TRUE(j.tentarLance(m))
                << "Lance da IA invalido: " << m
                << "\nFEN antes: " << fenAntes;
        }
        ++lance;
    }
}