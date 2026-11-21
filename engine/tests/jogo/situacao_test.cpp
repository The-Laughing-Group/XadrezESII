#include <gtest/gtest.h>
#include <string>
#include "xadrez/jogo.hpp"
#include "xadrez/posicao.hpp"
#include "tests/helpers_comuns.hpp" 


using namespace xadrez;


// situacao(): como está a partida
// =====================================================================

TEST(Situacao, PosicaoInicialEmAndamento) {
    Jogo j;
    const SituacaoPartida s = j.situacao();
    EXPECT_EQ(s.estado, "em_andamento");
    EXPECT_EQ(s.vencedor, "");
    EXPECT_FALSE(s.emXeque);
    EXPECT_FALSE(s.terminou);
}

TEST(Situacao, XequeSemFimDePartida) {
    Jogo j = carrega("4k3/8/8/8/8/8/4R3/4K3 b - - 0 1");
    const SituacaoPartida s = j.situacao();
    EXPECT_EQ(s.estado, "em_andamento");
    EXPECT_TRUE(s.emXeque);
    EXPECT_FALSE(s.terminou);
}

TEST(Situacao, XequeMateInformaOVencedor) {
    Jogo pretasMateadas = carrega("R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1");
    EXPECT_EQ(pretasMateadas.situacao().estado, "xeque_mate");
    EXPECT_EQ(pretasMateadas.situacao().vencedor, "brancas");
    EXPECT_TRUE(pretasMateadas.situacao().emXeque);
    EXPECT_TRUE(pretasMateadas.situacao().terminou);

    Jogo brancasMateadas = carrega("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    EXPECT_EQ(brancasMateadas.situacao().estado, "xeque_mate");
    EXPECT_EQ(brancasMateadas.situacao().vencedor, "pretas");
}

TEST(Situacao, Afogamento) {
    Jogo j = carrega("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    const SituacaoPartida s = j.situacao();
    EXPECT_EQ(s.estado, "afogamento");
    EXPECT_EQ(s.vencedor, "");
    EXPECT_FALSE(s.emXeque);
    EXPECT_TRUE(s.terminou);
}

TEST(Situacao, ConcordaComAsConsultasIndividuais) {
    const char* fens[] = {
        FEN_INICIAL,
        "R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1",
        "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",
        "4k3/8/8/8/8/8/4R3/4K3 b - - 0 1",
    };
    for (const char* fen : fens) {
        Jogo j = carrega(fen);
        const SituacaoPartida s = j.situacao();
        EXPECT_EQ(s.emXeque, j.emXeque()) << fen;
        EXPECT_EQ(s.estado == "xeque_mate", j.emXequeMate()) << fen;
        EXPECT_EQ(s.estado == "afogamento", j.emAfogamento()) << fen;
    }
}

TEST(Situacao, ConsultarNaoAlteraAPosicao) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    const std::string antes = j.fen();
    (void)j.situacao();
    (void)j.situacao();
    EXPECT_EQ(j.fen(), antes);
    EXPECT_EQ(j.ultimoLance().uci, "e2e4");
}

