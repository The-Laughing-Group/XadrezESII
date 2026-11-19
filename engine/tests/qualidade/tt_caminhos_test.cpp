// engine/tests/qualidade/tt_caminhos_test.cpp
// ler_tt (V=8), gravar_tt (V=6), pontuacao_para_tt / pontuacao_da_tt (V=3), iniciar_tt (V=1)
#include "helpers_motor.hpp"

class CC_TT : public th::LimpaTT {};

namespace {
constexpr std::uint64_t K  = 0x00DEADBEEF123456ULL;
constexpr std::uint64_t K2 = K ^ (1ULL << 60);   // mesmo slot, chave diferente
const Lance LM  = criar_lance(e2, e4, LANCE_AVANCO_DUPLO);
const Lance LM2 = criar_lance(g1, f3, LANCE_NORMAL);
const TTEntry& slot(std::uint64_t k) { return tabela_tt[k & tt_mascara]; }
}

TEST_F(CC_TT, IniciarTT_TamanhoPotenciaDeDois) {
    EXPECT_EQ((tt_mascara + 1) & tt_mascara, 0u);
    EXPECT_EQ(tabela_tt.size(), tt_mascara + 1);
}

// ---------------- ler_tt: um teste por caminho ----------------
TEST_F(CC_TT, Ler_P1_SlotVazio_NaoCorta) {
    int p = -777; Lance l = LM;
    EXPECT_FALSE(ler_tt(K, 3, 0, -100, 100, p, l));
    EXPECT_EQ(l, SEM_LANCE);
    EXPECT_EQ(p, -777);
}

TEST_F(CC_TT, Ler_P1b_ChaveDiferenteNoMesmoSlot_NaoCorta) {
    gravar_tt(K, 5, 0, 10, TTFlag::EXACT, LM);
    int p = 0; Lance l = LM2;
    EXPECT_FALSE(ler_tt(K2, 1, 0, -100, 100, p, l));
    EXPECT_EQ(l, SEM_LANCE);
}

TEST_F(CC_TT, Ler_P2_ProfundidadeInsuficiente_DevolveSoOLance) {
    gravar_tt(K, 4, 0, 50, TTFlag::EXACT, LM);
    int p = 0; Lance l = SEM_LANCE;
    EXPECT_FALSE(ler_tt(K, 5, 0, -100, 100, p, l));
    EXPECT_EQ(l, LM);
}

TEST_F(CC_TT, Ler_P3_Exact_CortaMesmoForaDaJanela) {
    gravar_tt(K, 4, 0, 50, TTFlag::EXACT, LM);
    int p = 0; Lance l = SEM_LANCE;
    EXPECT_TRUE(ler_tt(K, 4, 0, -10, 20, p, l));   // profundidade igual = acerto (fronteira)
    EXPECT_EQ(p, 50);
    EXPECT_EQ(l, LM);
}

TEST_F(CC_TT, Ler_P4_LowerBound_CortaQuandoPontuacaoMaiorOuIgualBeta) {
    gravar_tt(K, 4, 0, 100, TTFlag::LOWERBOUND, LM);
    int p = 0; Lance l;
    EXPECT_TRUE(ler_tt(K, 4, 0, -1000, 100, p, l));    // == beta
    EXPECT_EQ(p, 100);
    EXPECT_FALSE(ler_tt(K, 4, 0, -1000, 101, p, l));   // beta+1: nao corta (P5)
}

TEST_F(CC_TT, Ler_P6_UpperBound_CortaQuandoPontuacaoMenorOuIgualAlpha) {
    gravar_tt(K, 4, 0, 100, TTFlag::UPPERBOUND, LM);
    int p = 0; Lance l;
    EXPECT_TRUE(ler_tt(K, 4, 0, 100, 1000, p, l));     // == alpha
    EXPECT_EQ(p, 100);
    EXPECT_FALSE(ler_tt(K, 4, 0, 99, 1000, p, l));     // alpha-1: nao corta (P7)
}

TEST_F(CC_TT, Ler_P8_FlagsNaoSeConfundem) {
    int p = 0; Lance l;
    gravar_tt(K, 4, 0, 100, TTFlag::UPPERBOUND, LM);   // score >= beta, mas e UPPER
    EXPECT_FALSE(ler_tt(K, 4, 0, -1000, 50, p, l));
    gravar_tt(K, 4, 0, 100, TTFlag::LOWERBOUND, LM);   // score <= alpha, mas e LOWER
    EXPECT_FALSE(ler_tt(K, 4, 0, 200, 300, p, l));
}

TEST_F(CC_TT, Ler_Mate_ArmazenadoRelativoAoNo_LidoEmOutroPly) {
    gravar_tt(K, 6, 3, SCORE_MATE - 5, TTFlag::EXACT, LM);      // mate no ply 5, gravado no ply 3
    EXPECT_EQ(slot(K).pontuacao, SCORE_MATE - 2);               // "mate em 2 a partir do no"
    int p = 0; Lance l;
    ASSERT_TRUE(ler_tt(K, 6, 1, -SCORE_INFINITO, SCORE_INFINITO, p, l));
    EXPECT_EQ(p, SCORE_MATE - 3);                               // lido no ply 1
    gravar_tt(K + 1, 6, 3, -(SCORE_MATE - 5), TTFlag::EXACT, LM);
    ASSERT_TRUE(ler_tt(K + 1, 6, 1, -SCORE_INFINITO, SCORE_INFINITO, p, l));
    EXPECT_EQ(p, -(SCORE_MATE - 3));
}

// ---------------- gravar_tt: um teste por caminho ----------------
TEST_F(CC_TT, Gravar_G1_SlotVazio_Grava) {
    gravar_tt(K, 3, 0, 42, TTFlag::EXACT, LM);
    EXPECT_EQ(slot(K).chave, K);
    EXPECT_EQ(slot(K).profundidade, 3);
    EXPECT_EQ(slot(K).pontuacao, 42);
    EXPECT_EQ(slot(K).melhor_lance, LM);
    EXPECT_EQ(slot(K).idade, idade_atual);
}

TEST_F(CC_TT, Gravar_G2_MesmaPosicaoMesmaIdadeMenorProfundidade_NaoSubstitui) {
    gravar_tt(K, 6, 0, 42, TTFlag::EXACT, LM);
    gravar_tt(K, 5, 0, 99, TTFlag::LOWERBOUND, LM2);
    EXPECT_EQ(slot(K).pontuacao, 42);
    EXPECT_EQ(slot(K).profundidade, 6);
    EXPECT_EQ(slot(K).melhor_lance, LM);
}

TEST_F(CC_TT, Gravar_G3_MesmaProfundidade_Substitui_Fronteira) {
    gravar_tt(K, 6, 0, 42, TTFlag::EXACT, LM);
    gravar_tt(K, 6, 0, 99, TTFlag::LOWERBOUND, LM2);
    EXPECT_EQ(slot(K).pontuacao, 99);
    EXPECT_EQ(slot(K).flag, TTFlag::LOWERBOUND);
}

TEST_F(CC_TT, Gravar_G4_IdadeDiferente_SubstituiMesmoComMenorProfundidade) {
    gravar_tt(K, 6, 0, 42, TTFlag::EXACT, LM);
    nova_busca_tt();
    gravar_tt(K, 2, 0, 7, TTFlag::EXACT, LM2);
    EXPECT_EQ(slot(K).pontuacao, 7);
    EXPECT_EQ(slot(K).profundidade, 2);
    EXPECT_EQ(slot(K).idade, idade_atual);
}

TEST_F(CC_TT, Gravar_G5_ChaveDiferente_SubstituiMesmoComMenorProfundidade) {
    gravar_tt(K, 6, 0, 42, TTFlag::EXACT, LM);
    gravar_tt(K2, 1, 0, 5, TTFlag::EXACT, SEM_LANCE);
    EXPECT_EQ(slot(K).chave, K2);
    EXPECT_EQ(slot(K).melhor_lance, SEM_LANCE);   // outra posicao: NAO herda o lance antigo
}

TEST_F(CC_TT, Gravar_G6_MesmaPosicaoSemLanceNovo_PreservaLanceAntigo) {
    gravar_tt(K, 5, 0, 10, TTFlag::EXACT, LM);
    gravar_tt(K, 6, 0, 20, TTFlag::EXACT, SEM_LANCE);
    EXPECT_EQ(slot(K).pontuacao, 20);
    EXPECT_EQ(slot(K).melhor_lance, LM);
}

TEST_F(CC_TT, Gravar_G7_MesmaPosicaoComLanceNovo_UsaOLanceNovo) {
    gravar_tt(K, 5, 0, 10, TTFlag::EXACT, LM);
    gravar_tt(K, 6, 0, 20, TTFlag::EXACT, LM2);
    EXPECT_EQ(slot(K).melhor_lance, LM2);
}

// ---------------- conversoes de mate (constexpr, header) ----------------
TEST_F(CC_TT, ConversaoMate_TresCaminhosDeCadaFuncao) {
    const int ply = 7;
    EXPECT_EQ(pontuacao_para_tt(150, ply), 150);
    EXPECT_EQ(pontuacao_para_tt(SCORE_MATE - 3, ply), SCORE_MATE - 3 + ply);
    EXPECT_EQ(pontuacao_para_tt(-(SCORE_MATE - 3), ply), -(SCORE_MATE - 3) - ply);
    EXPECT_EQ(pontuacao_da_tt(150, ply), 150);
    EXPECT_EQ(pontuacao_da_tt(SCORE_MATE - 3, ply), SCORE_MATE - 3 - ply);
    EXPECT_EQ(pontuacao_da_tt(-(SCORE_MATE - 3), ply), -(SCORE_MATE - 3) + ply);
}

TEST_F(CC_TT, ConversaoMate_Fronteiras) {
    const int ply = 7;
    EXPECT_EQ(pontuacao_para_tt(SCORE_MATE_MIN, ply), SCORE_MATE_MIN + ply);
    EXPECT_EQ(pontuacao_para_tt(SCORE_MATE_MIN - 1, ply), SCORE_MATE_MIN - 1);
    EXPECT_EQ(pontuacao_para_tt(-SCORE_MATE_MIN, ply), -SCORE_MATE_MIN - ply);
    EXPECT_EQ(pontuacao_para_tt(-SCORE_MATE_MIN + 1, ply), -SCORE_MATE_MIN + 1);
    EXPECT_EQ(pontuacao_da_tt(SCORE_MATE_MIN, ply), SCORE_MATE_MIN - ply);
    EXPECT_EQ(pontuacao_da_tt(-SCORE_MATE_MIN, ply), -SCORE_MATE_MIN + ply);
}
