// engine/tests/qualidade/integracao_inter_test.cpp
// INTER-modulos: Busca alfa-beta + Avaliacao estatica + Tabela de Transposicao + Regras + Jogo
#include <algorithm>
#include <string>
#include "helpers_motor.hpp"
#include "../../../ia/busca-minimax/search.hpp"
#include "../../include/xadrez/jogo.hpp"
using namespace xadrez;

namespace { constexpr int INFI = SCORE_INFINITO; }

class Integracao_Inter : public th::LimpaTT {};

// ---- Busca <-> Avaliacao ----
TEST_F(Integracao_Inter, Avaliacao_ConvencaoDeSinal_PontuacaoEDasBrancas) {
    // A busca trata avaliar() como "pontos de vista das brancas" (brancas maximizam).
    // Se este teste falhar, avaliacao.hpp (que diz "de quem joga") e search.cpp divergem.
    Posicao w = Posicao::deFen("4k3/8/8/8/8/8/8/3QK3 w - - 0 1");
    Posicao b = Posicao::deFen("4k3/8/8/8/8/8/8/3QK3 b - - 0 1");
    EXPECT_GT(avaliar(w.b), 0);
    EXPECT_GT(avaliar(b.b), 0);
}

TEST_F(Integracao_Inter, Avaliacao_AntiSimetriaPorEspelho) {
    Posicao a = Posicao::deFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    Posicao e = Posicao::deFen("rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPPPPP/RNBQKBNR w KQkq e6 0 1");
    EXPECT_EQ(avaliar(a.b), -avaliar(e.b));
}

TEST_F(Integracao_Inter, Busca_Profundidade0_EhAAvaliacaoEstatica) {
    for (const char* fen : {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
                            "4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1"}) {
        Posicao p = Posicao::deFen(fen);
        const bool max = p.b.estado.lado_a_jogar == Cor::BRANCO;
        EXPECT_EQ(minimax_alfa_beta(p.b, 0, -INFI, INFI, max, 0), avaliar(p.b)) << fen;
    }
}

TEST_F(Integracao_Inter, Busca_PrefereGanharMaterial) {
    Posicao p = Posicao::deFen("4k3/8/8/3q4/8/8/8/3RK3 w - - 0 1");   // Txd5 ganha a dama
    EXPECT_EQ(th::paraUci(obter_melhor_lance(p.b, 3)), "d1d5");
}

// ---- Busca <-> TT ----
TEST_F(Integracao_Inter, BuscaTT_MateEmDois_TabelaFriaEQuenteDaoOMesmoLance) {
    Posicao p = Posicao::deFen("5q1k/6pp/8/8/8/6nP/8/6K1 b - - 0 1");
    EXPECT_EQ(th::paraUci(obter_melhor_lance(p.b, 4)), "f8f1");      // fria
    EXPECT_EQ(th::paraUci(obter_melhor_lance(p.b, 4)), "f8f1");      // quente (mesma TT)
    limpar_tt();
    EXPECT_EQ(th::paraUci(obter_melhor_lance(p.b, 5)), "f8f1");
}

TEST_F(Integracao_Inter, BuscaTT_PreencheATabelaEReduzNosNaSegundaBusca) {
    Posicao p = Posicao::deFen(FEN_INICIAL);
    (void)obter_melhor_lance(p.b, 3);
    const long long n1 = g_nos;
    const auto ocupadas = std::count_if(tabela_tt.begin(), tabela_tt.end(),
                                        [](const TTEntry& e) { return e.chave != 0; });
    EXPECT_GT(ocupadas, 0);
    (void)obter_melhor_lance(p.b, 3);
    EXPECT_LT(g_nos, n1) << "a TT quente deveria evitar reexpandir nos";
}

TEST_F(Integracao_Inter, BuscaTT_PontuacaoDeMateGravadaEmUmPlyELidaEmOutro) {
    Posicao p = Posicao::deFen("8/8/8/6pp/6pk/6pr/5PPP/5BKB b - - 0 1");
    EXPECT_EQ(minimax_alfa_beta(p.b, 2, -INFI, INFI, false, 0), -SCORE_MATE + 1);
    // mesma posicao, agora "vista" a 3 plies da raiz, com a TT ja preenchida
    EXPECT_EQ(minimax_alfa_beta(p.b, 2, -INFI, INFI, false, 3), -SCORE_MATE + 4);
}

// ---- Busca <-> Regras ----
TEST_F(Integracao_Inter, BuscaRegras_ReconheceEmpatePorRepeticaoDentroDaArvore) {
    // brancas com dama a mais, mas o historico ja repetiu a posicao: a busca devolve 0
    Posicao p = Posicao::deFen("4k3/8/8/8/8/8/8/3QK3 w - - 0 1");
    for (int i = 0; i < 2; ++i)
        for (const char* l : {"e1e2", "e8e7", "e2e1", "e7e8"}) ASSERT_TRUE(th::aplicar(p.b, l));
    EXPECT_EQ(minimax_alfa_beta(p.b, 2, -INFI, INFI, true, 0), 0);
}

// ---- Jogo <-> Regras ----
TEST(Integracao_Inter_Jogo, ContagemDeLancesDoJogoIgualAContagemDaInfra) {
    const char* fens[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};
    for (const char* fen : fens) {
        Jogo j;
        ASSERT_TRUE(j.carregarFen(fen));
        int viaJogo = 0;
        for (char c = 'a'; c <= 'h'; ++c)
            for (char r = '1'; r <= '8'; ++r) viaJogo += static_cast<int>(j.lancesDe(std::string{c, r}).size());
        Posicao p = Posicao::deFen(fen);
        EXPECT_EQ(viaJogo, th::contarLegais(p.b)) << fen;
    }
}

// ---- Jogo -> Busca -> Regras -> TT (ponta a ponta) ----
static std::string primeiroLance(const Jogo& j) {
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r) {
            auto v = j.lancesDe(std::string{c, r});
            if (!v.empty()) return v.front();
        }
    return "";
}

TEST(Integracao_Inter_Jogo, PartidaCurtaIAContraRoteiro_SemLancesIlegais) {
    Jogo j;
    for (int ply = 0; ply < 16 && !j.situacao().terminou; ++ply) {
        std::string m = j.vezDasBrancas() ? primeiroLance(j) : j.melhorLanceIA(3);
        ASSERT_FALSE(m.empty()) << "ply " << ply << " fen " << j.fen();
        ASSERT_TRUE(j.tentarLance(m)) << m << " em " << j.fen();
    }
}

TEST(Integracao_Inter_Jogo, MelhorLanceDaIA_SempreEstaEmLancesDe) {
    for (const char* fen : {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
                            "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
                            "4k3/8/8/8/8/8/4p3/K7 b - - 0 1"}) {
        Jogo j;
        ASSERT_TRUE(j.carregarFen(fen));
        const std::string m = j.melhorLanceIA(3);
        ASSERT_FALSE(m.empty()) << fen;
        const auto lances = j.lancesDe(m.substr(0, 2));
        EXPECT_NE(std::find(lances.begin(), lances.end(), m), lances.end()) << m << " em " << fen;
    }
}
