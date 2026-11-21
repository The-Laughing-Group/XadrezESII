#include <gtest/gtest.h>
#include <vector>
#include "xadrez/jogo.hpp"
#include "regras/helpers_regras.hpp"

using Jogo = xadrez::Jogo;


static const std::vector<CasoLances> kRoque = {
    {"RoqueCurtoDasBrancas",
     "4k3/8/8/8/8/8/8/4K2R w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2", "e1g1"}},
    {"RoqueLongoDasBrancas",
     "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"DoisRoquesDasBrancas",
     "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2", "e1g1"}},
    {"DoisRoquesDasPretas",
     "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", "e8",
     {"e8c8", "e8d7", "e8d8", "e8e7", "e8f7", "e8f8", "e8g8"}},
    {"SoUmDireitoSoUmRoque",
     "r3k2r/8/8/8/8/8/8/4K3 b k - 0 1", "e8",
     {"e8d7", "e8d8", "e8e7", "e8f7", "e8f8", "e8g8"}},
    {"SemDireitoNaoGeraRoque",
     "4k3/8/8/8/8/8/8/R3K2R w - - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"CurtoBloqueadoPorPecaEntreReiETorre",
     "4k3/8/8/8/8/8/8/4KN1R w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f2"}},
    {"CurtoBloqueadoPorPecaAdversariaEmG1",
     "4k3/8/8/8/8/8/8/4K1nR w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1f1", "e1f2"}},
    {"LongoBloqueadoPorPecaEmD1",
     "4k3/8/8/8/8/8/8/R2BK3 w Q - 0 1", "e1",
     {"e1d2", "e1e2", "e1f1", "e1f2"}},
    {"LongoBloqueadoPorPecaEmC1",
     "4k3/8/8/8/8/8/8/R1B1K3 w Q - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"LongoPrecisaDaCasaBLivre",
     "4k3/8/8/8/8/8/8/RB2K3 w Q - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"SemATorreNoCantoNaoGera",
     "4k3/8/8/8/8/8/8/4K3 w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"BispoAdversarioNoCantoNaoServe",
     "4k3/8/8/8/8/8/8/4K2b w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"ReiForaDaCasaInicialNaoFazRoque",
     "4k3/8/8/8/8/8/8/3K3R w K - 0 1", "d1",
     {"d1c1", "d1c2", "d1d2", "d1e1", "d1e2"}},
    {"ReiNaColunaHNaoGeraLanceExtra",
     "4k3/8/8/8/8/8/8/7K w KQ - 0 1", "h1",
     {"h1g1", "h1g2", "h1h2"}},
    {"ReiNaColunaGNaoGeraLanceExtra",
     "4k3/8/8/8/8/8/8/6K1 w KQ - 0 1", "g1",
     {"g1f1", "g1f2", "g1g2", "g1h1", "g1h2"}},
    {"ReiBrancoNaFileiraOitoNaoFazRoque",
     "R3K2R/8/8/8/8/8/8/4k3 w KQ - 0 1", "e8",
     {"e8d7", "e8d8", "e8e7", "e8f7", "e8f8"}},
    {"CasaBAtacadaNaoImpedeORoqueLongo",
     "1r6/6k1/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2", "e1g1"}},
    {"TorreAtacadaNaoImpedeORoque",
     "6k1/8/2b5/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2", "e1g1"}},
    {"ReiEmXequeNaoFazRoque",
     "1k6/8/8/4r3/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1d1", "e1d2", "e1f1", "e1f2"}},
    {"ReiNaoPassaPorCasaAtacadaNoRoqueCurto",
     "1k3r2/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2"}},
    {"ReiNaoPassaPorCasaAtacadaNoRoqueLongo",
     "1k1r4/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1e2", "e1f1", "e1f2", "e1g1"}},
    {"ReiNaoTerminaEmCasaAtacada",
     "1k4r1/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"PretasTambemRespeitamCasasAtacadas",
     "r3k2r/8/8/8/8/8/8/4KR2 b kq - 0 1", "e8",
     {"e8c8", "e8d7", "e8d8", "e8e7"}},
};
INSTANTIATE_TEST_SUITE_P(Roque, LancesExatos, ::testing::ValuesIn(kRoque), nomeDoCaso);

// roque executado pelo Jogo (gerar + validar + aplicar)

TEST(Movimentos, RoqueNoJogoListaOsDoisRoques) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    auto v = j.lancesDe("e1");
    EXPECT_TRUE(contem(v, "e1g1"));
    EXPECT_TRUE(contem(v, "e1c1"));
}

TEST(Movimentos, RoqueNoJogoCurtoDasBrancasMoveReiETorre) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    EXPECT_TRUE(j.tentarLance("e1g1"));
    EXPECT_EQ(j.fen(), "4k3/8/8/8/8/8/8/R4RK1 b - - 1 1");   // perdeu os dois direitos
}

TEST(Movimentos, RoqueNoJogoCurtoDasPretasMoveReiETorre) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1"));
    EXPECT_TRUE(j.tentarLance("e8g8"));
    EXPECT_EQ(j.fen(), "r4rk1/8/8/8/8/8/8/4K3 w - - 1 2");
}

TEST(Movimentos, RoqueNoJogoLongoDasPretasMoveReiETorre) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1"));
    EXPECT_TRUE(j.tentarLance("e8c8"));
    EXPECT_EQ(j.fen(), "2kr3r/8/8/8/8/8/8/4K3 w - - 1 2");
}

TEST(Movimentos, RoqueNoJogoTorreMovidaTiraSoOSeuRoque) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    ASSERT_TRUE(j.tentarLance("h1h2"));      // brancas
    ASSERT_TRUE(j.tentarLance("e8d8"));      // pretas
    auto v = j.lancesDe("e1");
    EXPECT_FALSE(contem(v, "e1g1"));         // a torre h já saiu
    EXPECT_TRUE(contem(v, "e1c1"));          // a torre a continua valendo
}

TEST(Movimentos, RoqueNoJogoReiMovidoNaoVoltaAFazerRoque) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    ASSERT_TRUE(j.tentarLance("e1e2"));
    ASSERT_TRUE(j.tentarLance("e8d8"));
    ASSERT_TRUE(j.tentarLance("e2e1"));      // o rei volta para a casa inicial
    ASSERT_TRUE(j.tentarLance("d8e8"));
    auto v = j.lancesDe("e1");
    EXPECT_FALSE(contem(v, "e1g1"));
    EXPECT_FALSE(contem(v, "e1c1"));
}

TEST(Movimentos, RoqueNoJogoTorreCapturadaNoCantoTiraORoqueDoDono) {
    Jogo j;
    // a torre preta sai de h8 e captura a torre branca em h1:
    // as brancas perdem o roque curto (K) e as pretas também (k), porque a torre delas saiu de h8
    ASSERT_TRUE(j.carregarFen("4k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1"));
    ASSERT_TRUE(j.tentarLance("h8h1"));
    EXPECT_EQ(j.fen(), "4k3/8/8/8/8/8/8/R3K2r w Q - 0 2");
}

TEST(Movimentos, RoqueNoJogoEhRecusadoQuandoOReiPassaPorCasaAtacada) {
    Jogo j;
    // torre preta em f8 ataca f1: o roque curto é ilegal, o longo continua valendo
    ASSERT_TRUE(j.carregarFen("1k3r2/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    const std::string antes = j.fen();
    EXPECT_FALSE(j.tentarLance("e1g1"));
    EXPECT_EQ(j.fen(), antes);
    EXPECT_TRUE(j.tentarLance("e1c1"));
}