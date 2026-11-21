#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace xadrez;

// Convenção: Jogo::tabuleiro() devolve '.' para casa vazia e uma linha por fileira,
// da 8 (índice 0) à 1 (índice 7). Se jogo.cpp usar outro caractere, o ajuste é em
// jogo.cpp E nos literais deste arquivo.

static bool contem(const std::vector<std::string>& v, const std::string& item) {
    return std::find(v.begin(), v.end(), item) != v.end();
}

// Carrega o FEN, tenta o lance e confere que foi recusado SEM alterar a posição.
static void esperaRecusa(const char* fen, const char* uci) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen(fen)) << fen;
    const std::string antes = j.fen();
    EXPECT_FALSE(j.tentarLance(uci)) << uci << " em " << fen;
    EXPECT_EQ(j.fen(), antes) << "a posição mudou apesar da recusa de " << uci;
}

// ---------- posição inicial e leitura ----------

TEST(Jogo, ComecaNaPosicaoInicial) {
    Jogo j;
    EXPECT_EQ(j.fen(), FEN_INICIAL);
    EXPECT_TRUE(j.vezDasBrancas());
}

TEST(Jogo, TabuleiroDevolveOitoLinhasDeOitoCaracteres) {
    Jogo j;
    std::vector<std::string> t = j.tabuleiro();
    ASSERT_EQ(t.size(), 8u);
    for (const auto& linha : t) EXPECT_EQ(linha.size(), 8u);
    EXPECT_EQ(t[0], "rnbqkbnr");
    EXPECT_EQ(t[1], "pppppppp");
    EXPECT_EQ(t[2], "........");
    EXPECT_EQ(t[5], "........");
    EXPECT_EQ(t[6], "PPPPPPPP");
    EXPECT_EQ(t[7], "RNBQKBNR");
}

TEST(Jogo, CarregarFenValidoSubstituiAPosicao) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    ASSERT_TRUE(j.carregarFen("k7/4P3/8/8/8/8/8/4K3 w - - 0 1"));
    EXPECT_EQ(j.fen(), "k7/4P3/8/8/8/8/8/4K3 w - - 0 1");
    EXPECT_TRUE(j.vezDasBrancas());
    EXPECT_EQ(j.tabuleiro()[1], "....P...");
}

TEST(Jogo, FenInvalidoDevolveFalseEMantemAPosicao) {
    Jogo j;
    EXPECT_FALSE(j.carregarFen("isso nao e um fen"));
    EXPECT_FALSE(j.carregarFen("8/8/8/8/8/8/8/8 w - - 0 1"));   // sem reis
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}

TEST(Jogo, FenInvalidoMantemAPosicaoAtualEnaoVoltaParaAInicial) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    const std::string antes = j.fen();
    EXPECT_FALSE(j.carregarFen("isso nao e um fen"));
    EXPECT_FALSE(j.carregarFen("8/8/8/8/8/8/8/8 w - - 0 1"));
    EXPECT_EQ(j.fen(), antes);
    EXPECT_FALSE(j.vezDasBrancas());
}

TEST(Jogo, CopiaEhIndependenteDoOriginal) {
    Jogo a;
    Jogo b = a;
    ASSERT_TRUE(b.tentarLance("e2e4"));
    EXPECT_EQ(a.fen(), FEN_INICIAL);
    EXPECT_TRUE(a.vezDasBrancas());
    EXPECT_NE(b.fen(), FEN_INICIAL);
}

// ---------- lancesDe ----------

TEST(Jogo, LancesDoPeaoNaPosicaoInicial) {
    Jogo j;
    std::vector<std::string> v = j.lancesDe("e2");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(contem(v, "e2e3"));
    EXPECT_TRUE(contem(v, "e2e4"));
}

TEST(Jogo, LancesDoCavaloNaPosicaoInicial) {
    Jogo j;
    std::vector<std::string> v = j.lancesDe("g1");
    EXPECT_EQ(v.size(), 2u);   // e2 está ocupada por peão próprio
    EXPECT_TRUE(contem(v, "g1f3"));
    EXPECT_TRUE(contem(v, "g1h3"));
}

TEST(Jogo, PecaDeQuemNaoTemAVezNaoTemLances) {
    Jogo j;   // vez das brancas
    EXPECT_TRUE(j.lancesDe("e7").empty());
    EXPECT_TRUE(j.lancesDe("g8").empty());
}

TEST(Jogo, CasaVaziaOuInvalidaNaoTemLances) {
    Jogo j;
    EXPECT_TRUE(j.lancesDe("e4").empty());
    EXPECT_TRUE(j.lancesDe("i9").empty());
    EXPECT_TRUE(j.lancesDe("a0").empty());
    EXPECT_TRUE(j.lancesDe("").empty());
    EXPECT_TRUE(j.lancesDe("e22").empty());
    EXPECT_TRUE(j.lancesDe("e2e4").empty());
}

TEST(Jogo, PromocaoSaiComALetraNoFinal) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("k7/4P3/8/8/8/8/8/4K3 w - - 0 1"));
    std::vector<std::string> v = j.lancesDe("e7");
    EXPECT_EQ(v.size(), 4u);
    EXPECT_TRUE(contem(v, "e7e8q"));
    EXPECT_TRUE(contem(v, "e7e8r"));
    EXPECT_TRUE(contem(v, "e7e8b"));
    EXPECT_TRUE(contem(v, "e7e8n"));
}

TEST(Jogo, VezDasPretasLiberaOsLancesDasPretas) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"));
    EXPECT_FALSE(j.vezDasBrancas());
    EXPECT_EQ(j.lancesDe("e7").size(), 2u);
    EXPECT_TRUE(j.lancesDe("e4").empty());   // peça das brancas
}

TEST(Jogo, LancesMudamDepoisDeJogar) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    EXPECT_TRUE(j.lancesDe("e2").empty());          // a casa ficou vazia
    EXPECT_TRUE(j.lancesDe("e4").empty());          // peça das brancas, mas a vez é das pretas
    EXPECT_EQ(j.lancesDe("e7").size(), 2u);         // agora as pretas podem mover
}

// ---------- tentarLance: lances válidos ----------

TEST(Jogo, TentarLanceValidoAtualizaAPosicao) {
    Jogo j;
    EXPECT_TRUE(j.tentarLance("e2e4"));
    EXPECT_EQ(j.fen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    EXPECT_FALSE(j.vezDasBrancas());
    EXPECT_EQ(j.tabuleiro()[4], "....P...");
    EXPECT_EQ(j.tabuleiro()[6], "PPPP.PPP");
}

TEST(Jogo, SequenciaDeLancesAlternaAVez) {
    Jogo j;
    EXPECT_TRUE(j.tentarLance("e2e4"));
    EXPECT_TRUE(j.tentarLance("e7e5"));
    EXPECT_TRUE(j.tentarLance("g1f3"));
    EXPECT_TRUE(j.tentarLance("b8c6"));
    EXPECT_EQ(j.fen(), "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3");
    EXPECT_FALSE(j.tentarLance("b8c6"));   // as pretas não jogam duas vezes seguidas
}

TEST(Jogo, PartidaComRoqueAteOSextoLance) {
    // Ruy Lopez: 1.e4 e5 2.Cf3 Cc6 3.Bb5 a6 4.Ba4 Cf6 5.O-O Be7
    Jogo j;
    for (const char* lance : {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6",
                              "b5a4", "g8f6", "e1g1", "f8e7"})
        ASSERT_TRUE(j.tentarLance(lance)) << lance;
    EXPECT_EQ(j.fen(), "r1bqk2r/1pppbppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQ1RK1 w kq - 4 6");
}

// ---------- tentarLance: lances inválidos não mudam nada ----------

TEST(Jogo, TentarLanceInvalidoNaoMudaNada) {
    Jogo j;
    EXPECT_FALSE(j.tentarLance("e2e5"));   // peão não anda 3 casas
    EXPECT_FALSE(j.tentarLance("e2d3"));   // diagonal sem captura
    EXPECT_FALSE(j.tentarLance("a1a3"));   // torre bloqueada pelo próprio peão
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}

TEST(Jogo, TentarLanceDeCasaVaziaOuDePecaDoAdversario) {
    Jogo j;
    EXPECT_FALSE(j.tentarLance("e4e5"));   // casa vazia
    EXPECT_FALSE(j.tentarLance("e7e5"));   // peça preta na vez das brancas
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}

TEST(Jogo, TentarLanceComTextoMalFormado) {
    Jogo j;
    EXPECT_FALSE(j.tentarLance(""));
    EXPECT_FALSE(j.tentarLance("e2"));
    EXPECT_FALSE(j.tentarLance("e2e4e4"));
    EXPECT_FALSE(j.tentarLance("z9z9"));
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}

// Formato estrito: só UCI puro ("e2e4", "e7e8q"). Se por decisão de projeto o Jogo
// passar a aceitar espaços ou hífen, remova os casos correspondentes.
TEST(Jogo, TentarLanceRejeitaFormatosQueNaoSaoUci) {
    Jogo j;
    for (const char* texto : {"e2e2", "e2e4q", "e2-e4", "e2e4 ", " e2e4", "0000", "e2e", "2e4e"})
        EXPECT_FALSE(j.tentarLance(texto)) << "'" << texto << "'";
    EXPECT_EQ(j.fen(), FEN_INICIAL);

    Jogo p;
    ASSERT_TRUE(p.carregarFen("k7/4P3/8/8/8/8/8/4K3 w - - 0 1"));
    const std::string antes = p.fen();
    EXPECT_FALSE(p.tentarLance("e7e8x"));
    EXPECT_FALSE(p.tentarLance("e7e8qq"));
    EXPECT_EQ(p.fen(), antes);
}

// ---------- tentarLance: legalidade (rei não pode ficar em xeque) ----------

TEST(Jogo, PecaCravadaNaoPodeSairDaLinha) {
    esperaRecusa("4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1", "e2d2");
}

TEST(Jogo, ReiNaoPodeEntrarEmCasaAtacada) {
    esperaRecusa("4k3/8/8/8/8/8/r7/4K3 w - - 0 1", "e1e2");
}

TEST(Jogo, EmXequeUmLanceQueNaoResolveEhRecusado) {
    esperaRecusa("k3r3/8/8/8/8/8/8/1N2K3 w - - 0 1", "b1c3");   // o cavalo não bloqueia a coluna e
}

TEST(Jogo, EmXequeUmLanceQueBloqueiaEhAceito) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("1k2r3/8/8/8/R7/8/8/4K3 w - - 0 1"));
    EXPECT_TRUE(j.tentarLance("a4e4"));
}

TEST(Jogo, RoqueAtravesDeCasaAtacadaEhRecusado) {
    esperaRecusa("1k3r2/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1g1");
}

TEST(Jogo, EnPassantQueExpoeOReiEhRecusado) {
    esperaRecusa("8/8/8/8/k2Pp2Q/8/8/3K4 b - d3 0 1", "e4d3");
}

// ---------- tentarLance: promoção ----------

TEST(Jogo, PromocaoExigeAPecaEscolhida) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("k7/4P3/8/8/8/8/8/4K3 w - - 0 1"));
    EXPECT_FALSE(j.tentarLance("e7e8"));            // falta dizer em que peça promove
    EXPECT_FALSE(j.tentarLance("e7e8k"));           // peão não vira rei
    EXPECT_TRUE(j.tentarLance("e7e8Q"));            // maiúscula também é aceita
    EXPECT_EQ(j.fen(), "k3Q3/8/8/8/8/8/8/4K3 b - - 0 1");
}

TEST(Jogo, PromocaoDasPretasComLetraMaiusculaGeraPecaPreta) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/4p3/K7 b - - 0 1"));
    EXPECT_TRUE(j.tentarLance("e2e1Q"));
    EXPECT_EQ(j.fen(), "4k3/8/8/8/8/8/8/K3q3 w - - 0 2");   // dama PRETA (minúscula no FEN)
}

TEST(Jogo, PromocaoComCapturaPeloJogo) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1"));
    EXPECT_TRUE(j.tentarLance("e7d8q"));
    EXPECT_EQ(j.fen(), "k2Q4/8/8/8/8/8/8/4K3 b - - 0 1");
}

// ---------- en passant ----------

TEST(Jogo, EnPassantPeloJogo) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1"));
    EXPECT_TRUE(j.tentarLance("e5d6"));
    EXPECT_EQ(j.fen(), "4k3/8/3P4/8/8/8/8/4K3 b - - 0 1");
}

TEST(Jogo, TabuleiroRefleteOEnPassant) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1"));
    ASSERT_TRUE(j.tentarLance("e5d6"));
    std::vector<std::string> t = j.tabuleiro();
    ASSERT_EQ(t.size(), 8u);
    EXPECT_EQ(t[2], "...P....");   // o peão pousou em d6
    EXPECT_EQ(t[3], "........");   // o peão preto de d5 foi removido
    EXPECT_EQ(t[4], "........");   // e5 ficou vazia
}

TEST(Jogo, EnPassantEmPartidaCompleta) {
    Jogo j;
    for (const char* lance : {"e2e4", "a7a6", "e4e5", "d7d5", "e5d6"})
        ASSERT_TRUE(j.tentarLance(lance)) << lance;
    EXPECT_EQ(j.fen(), "rnbqkbnr/1pp1pppp/p2P4/8/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3");
}

TEST(Jogo, EnPassantExpiraSeNaoForUsadoNaMesmaHora) {
    Jogo j;
    for (const char* lance : {"e2e4", "a7a6", "e4e5", "d7d5", "g1f3", "h7h6"})
        ASSERT_TRUE(j.tentarLance(lance)) << lance;
    const std::string antes = j.fen();
    EXPECT_FALSE(j.tentarLance("e5d6"));   // a oportunidade passou
    EXPECT_EQ(j.fen(), antes);
}