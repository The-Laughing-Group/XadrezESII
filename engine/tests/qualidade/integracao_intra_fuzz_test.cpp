// engine/tests/qualidade/integracao_intra_fuzz_test.cpp
// INTRA-modulo (property test com semente fixa). Complementa integracao_intra_test.cpp.
// Compara explicitamente (o assert dentro de fazer_lance some em Release).
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>
#include "helpers_motor.hpp"
using namespace xadrez;

TEST(Integracao_Intra_Fuzz, PartidasAleatorias_ChaveConsistenciaEFen) {
    std::mt19937 rng(12345);
    for (int g = 0; g < 60; ++g) {
        Posicao p = Posicao::inicial();
        for (int ply = 0; ply < 300; ++ply) {
            ListaLances lista;
            gerar_lances(p.b, lista);
            std::vector<int> ordem(lista.quantidade);
            std::iota(ordem.begin(), ordem.end(), 0);
            std::shuffle(ordem.begin(), ordem.end(), rng);

            bool jogou = false;
            for (int i : ordem)
                if (fazer_lance(p.b, lista.lances[i])) { jogou = true; break; }
            if (!jogou) break;                                   // mate ou afogamento

            const std::string fen = p.paraFen();
            ASSERT_TRUE(verificar_consistencia(p.b)) << "jogo " << g << " ply " << ply << " " << fen;
            ASSERT_EQ(p.b.estado.chave_zobrist, calcular_chave(p.b))
                << "chave incremental divergiu. jogo " << g << " ply " << ply << " " << fen;

            Posicao q = Posicao::deFen(fen);                      // ida e volta pelo FEN
            ASSERT_EQ(q.paraFen(), fen);
            ASSERT_EQ(q.b.estado.chave_zobrist, p.b.estado.chave_zobrist)
                << "chave via FEN difere da incremental: " << fen;
            ASSERT_EQ(th::contarLegais(q.b), th::contarLegais(p.b))
                << "lances legais mudam apos recarregar o FEN: " << fen;
        }
    }
}