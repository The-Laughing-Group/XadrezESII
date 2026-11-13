// engine/tests/qualidade/busca_caminhos_test.cpp
// minimax_alfa_beta (V=23) e obter_melhor_lance (V=9)
#include <algorithm>
#include "helpers_motor.hpp"
#include "../../../ia/busca-minimax/search.hpp"
using namespace xadrez;

namespace { constexpr int INFI = SCORE_INFINITO; }

class CC_Busca : public th::LimpaTT {};

// ---- folha / empates por regra ----
TEST_F(CC_Busca, Minimax_B1_Empate50Retorna0) {
    Posicao p = Posicao::deFen("4k3/8/8/8/8/8/R7/4K3 w - - 100 80");
    EXPECT_EQ(minimax_alfa_beta(p.b, 3, -INFI, INFI, true, 0), 0);
}

TEST_F(CC_Busca, Minimax_B2_RepeticaoRetorna0) {
    Posicao p = Posicao::deFen(FEN_INICIAL);
    for (int i = 0; i < 2; ++i)
        for (const char* l : {"g1f3", "g8f6", "f3g1", "f6g8"}) ASSERT_TRUE(th::aplicar(p.b, l));
    EXPECT_EQ(minimax_alfa_beta(p.b, 3, -INFI, INFI, true, 0), 0);
}

TEST_F(CC_Busca, Minimax_B3_FolhaSemXeque_IgualAvaliar) {
    Posicao p = Posicao::deFen("4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1");
    EXPECT_EQ(minimax_alfa_beta(p.b, 0, -INFI, INFI, true, 0), avaliar(p.b));
}

TEST_F(CC_Busca, Minimax_B4_FolhaEmXequeSemSerMate_IgualAvaliar) {
    Posicao p = Posicao::deFen("4k3/8/8/8/8/8/4R3/4K3 b - - 0 1");
    EXPECT_EQ(minimax_alfa_beta(p.b, 0, -INFI, INFI, false, 0), avaliar(p.b));
}

TEST_F(CC_Busca, Minimax_B5_FolhaMate_PretasMateadas) {
    Posicao p = Posicao::deFen("R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1");
    EXPECT_EQ(minimax_alfa_beta(p.b, 0, -INFI, INFI, false, 3), SCORE_MATE - 3);
}

TEST_F(CC_Busca, Minimax_B6_FolhaMate_BrancasMateadas) {
    Posicao p = Posicao::deFen("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    EXPECT_EQ(minimax_alfa_beta(p.b, 0, -INFI, INFI, true, 3), -SCORE_MATE + 3);
}

TEST_F(CC_Busca, Minimax_B7_SemLancesLegais_MateEAfogamento) {
    Posicao m = Posicao::deFen("R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1");
    EXPECT_EQ(minimax_alfa_beta(m.b, 2, -INFI, INFI, false, 2), SCORE_MATE - 2);
    Posicao a = Posicao::deFen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    EXPECT_EQ(minimax_alfa_beta(a.b, 2, -INFI, INFI, false, 2), 0);
}

// ---- tabela de transposicao ----
TEST_F(CC_Busca, Minimax_T1_AcertoExato) {
    Posicao p = Posicao::deFen(FEN_INICIAL);
    gravar_tt(p.b.estado.chave_zobrist, 6, 0, 1234, TTFlag::EXACT, SEM_LANCE);
    EXPECT_EQ(minimax_alfa_beta(p.b, 3, -INFI, INFI, true, 0), 1234);
}

TEST_F(CC_Busca, Minimax_T2_AcertoLowerBound) {
    Posicao p = Posicao::deFen(FEN_INICIAL);
    gravar_tt(p.b.estado.chave_zobrist, 6, 0, 500, TTFlag::LOWERBOUND, SEM_LANCE);
    EXPECT_EQ(minimax_alfa_beta(p.b, 3, -INFI, 100, true, 0), 500);
}

TEST_F(CC_Busca, Minimax_T3_AcertoUpperBound) {
    Posicao p = Posicao::deFen(FEN_INICIAL);
    gravar_tt(p.b.estado.chave_zobrist, 6, 0, -500, TTFlag::UPPERBOUND, SEM_LANCE);
    EXPECT_EQ(minimax_alfa_beta(p.b, 3, 100, INFI, true, 0), -500);
}

TEST_F(CC_Busca, Minimax_T4_DicaDeLanceNaoMudaOValor) {
    Posicao p = Posicao::deFen(FEN_INICIAL);
    const int base = minimax_alfa_beta(p.b, 3, -INFI, INFI, true, 0);

    limpar_tt();
    gravar_tt(p.b.estado.chave_zobrist, 0, 0, 0, TTFlag::EXACT, th::acharLance(p.b, "a2a3"));
    EXPECT_EQ(minimax_alfa_beta(p.b, 3, -INFI, INFI, true, 0), base);   // dica presente na lista

    limpar_tt();
    gravar_tt(p.b.estado.chave_zobrist, 0, 0, 0, TTFlag::EXACT, criar_lance(h8, a1, LANCE_NORMAL));
    EXPECT_EQ(minimax_alfa_beta(p.b, 3, -INFI, INFI, true, 0), base);   // dica ausente da lista
}

// ---- alfa-beta: janelas ----
TEST_F(CC_Busca, Minimax_A1_JanelaEstreita_FailLowEFailHigh) {
    Posicao p = Posicao::deFen("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
    auto busca = [&](int a, int b) { limpar_tt(); return minimax_alfa_beta(p.b, 3, a, b, true, 0); };
    const int v = busca(-INFI, INFI);
    EXPECT_EQ(busca(v - 5, v + 5), v);
    const int baixo = busca(v + 10, v + 20);
    EXPECT_LE(baixo, v + 10); EXPECT_GE(baixo, v);
    const int alto = busca(v - 20, v - 10);
    EXPECT_GE(alto, v - 10);  EXPECT_LE(alto, v);
}

TEST_F(CC_Busca, Minimax_A2_MaximizadorEMinimizadorSimetricos) {
    // Posicao e seu espelho (fileiras invertidas, cores trocadas): valores opostos
    Posicao a = Posicao::deFen("4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1");
    Posicao b = Posicao::deFen("4k3/8/8/4p3/3P4/8/8/4K3 b - - 0 1");
    const int va = minimax_alfa_beta(a.b, 3, -INFI, INFI, true, 0);
    limpar_tt();
    const int vb = minimax_alfa_beta(b.b, 3, -INFI, INFI, false, 0);
    EXPECT_EQ(va, -vb);
}

// ---- mate: pontuacao e TT quente ----
TEST_F(CC_Busca, Minimax_M1_MateEmUm_PretasPontuamNegativo) {
    Posicao p = Posicao::deFen("8/8/8/6pp/6pk/6pr/5PPP/5BKB b - - 0 1");
    EXPECT_EQ(minimax_alfa_beta(p.b, 2, -INFI, INFI, false, 0), -SCORE_MATE + 1);
}

TEST_F(CC_Busca, Minimax_M2_MateEmDois_EstavelComTTQuente) {
    Posicao p = Posicao::deFen("5q1k/6pp/8/8/8/6nP/8/6K1 b - - 0 1");
    EXPECT_EQ(minimax_alfa_beta(p.b, 4, -INFI, INFI, false, 0), -SCORE_MATE + 3);
    EXPECT_EQ(minimax_alfa_beta(p.b, 5, -INFI, INFI, false, 0), -SCORE_MATE + 3);  // sem limpar a TT
}

// ---- obter_melhor_lance ----
TEST_F(CC_Busca, Raiz_R1_SemLanceLegal_DevolveSemLance) {
    Posicao a = Posicao::deFen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    EXPECT_EQ(obter_melhor_lance(a.b, 3), SEM_LANCE);
    Posicao m = Posicao::deFen("R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1");
    EXPECT_EQ(obter_melhor_lance(m.b, 3), SEM_LANCE);
}

TEST_F(CC_Busca, Raiz_R2_Pretas_EscolheMateEmUm) {
    Posicao p = Posicao::deFen("8/8/8/6pp/6pk/6pr/5PPP/5BKB b - - 0 1");
    EXPECT_EQ(th::paraUci(obter_melhor_lance(p.b, 3)), "g3h2");
}

TEST_F(CC_Busca, Raiz_R3_Brancas_EscolheMateEmUm) {
    Posicao p = Posicao::deFen("6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1");
    EXPECT_EQ(th::paraUci(obter_melhor_lance(p.b, 3)), "a1a8");
}

TEST_F(CC_Busca, Raiz_R4_NuncaDevolveLanceIlegal) {
    Posicao p = Posicao::deFen("4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1");   // torre e2 cravada
    const Lance m = obter_melhor_lance(p.b, 3);
    ASSERT_NE(m, SEM_LANCE);
    EXPECT_NE(th::paraUci(m), "e2d2");
    EXPECT_TRUE(fazer_lance(p.b, m));
}

TEST_F(CC_Busca, Raiz_R5_TabuleiroIntactoDepoisDaBusca) {
    Posicao p = Posicao::deFen(FEN_INICIAL);
    auto antes = std::make_unique<Tabuleiro>(p.b);
    (void)obter_melhor_lance(p.b, 3);
    EXPECT_TRUE(th::iguais(*antes, p.b));
}

TEST_F(CC_Busca, Raiz_R6_ModoDebugRegistraLog) {
    Posicao p = Posicao::deFen("8/8/8/6pp/6pk/6pr/5PPP/5BKB b - - 0 1");
    const bool antes = g_debug_busca;
    g_debug_busca = true;
    g_log_busca.str(""); g_log_busca.clear();
    const Lance m = obter_melhor_lance(p.b, 2);
    g_debug_busca = antes;
    const std::string log = g_log_busca.str();
    g_log_busca.str(""); g_log_busca.clear();
    EXPECT_NE(m, SEM_LANCE);
    EXPECT_NE(log.find("melhor: g3h2"), std::string::npos);
    EXPECT_NE(log.find("nos:"), std::string::npos);
}
