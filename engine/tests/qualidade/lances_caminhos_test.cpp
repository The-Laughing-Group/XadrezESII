// engine/tests/qualidade/lances_caminhos_test.cpp
// fazer_lance (V=17), desfazer_lance (V=12), novos_direitos (V=8, via fazer_lance)
#include "helpers_motor.hpp"
using namespace xadrez;

namespace {
const char* START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char* ROQ_W = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
const char* ROQ_B = "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1";
}

// ---------- ida e volta: cada linha percorre um caminho distinto de fazer/desfazer ----------
struct CasoLance { const char* nome; const char* fen; const char* uci; };

class CC_Lances_RoundTrip : public ::testing::TestWithParam<CasoLance> {};

TEST_P(CC_Lances_RoundTrip, FazerEDesfazerRestauraTudo) {
    const CasoLance& c = GetParam();
    Posicao p = Posicao::deFen(c.fen);
    auto antes = std::make_unique<Tabuleiro>(p.b);
    const Lance m = th::acharLance(p.b, c.uci);
    ASSERT_NE(m, SEM_LANCE);
    ASSERT_TRUE(fazer_lance(p.b, m)) << c.fen << " " << c.uci;
    EXPECT_TRUE(verificar_consistencia(p.b));
    EXPECT_EQ(p.b.estado.chave_zobrist, calcular_chave(p.b)) << "Zobrist incremental divergiu";
    EXPECT_EQ(p.b.conta_lance, antes->conta_lance + 1);
    EXPECT_EQ(p.b.estado.lado_a_jogar, oposta(antes->estado.lado_a_jogar));
    desfazer_lance(p.b, m);
    EXPECT_TRUE(verificar_consistencia(p.b));
    EXPECT_TRUE(th::iguais(*antes, p.b));
}

static const CasoLance kRoundTrip[] = {
    {"Quieto_Cavalo",          START, "g1f3"},
    {"Peao_Simples",           START, "e2e3"},
    {"AvancoDuplo",            START, "e2e4"},
    {"Captura_Cavalo",         "4k3/8/8/3p4/8/2N5/8/4K3 w - - 5 10", "c3d5"},
    {"Captura_Peao",           "4k3/8/8/3p4/4P3/8/8/4K3 w - - 5 10", "e4d5"},
    {"EnPassant_Branco",       "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5d6"},
    {"EnPassant_Preto",        "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1", "d4e3"},
    {"Promocao_Branca",        "k7/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7e8q"},
    {"Promocao_ComCaptura",    "k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7d8n"},
    {"Promocao_Preta",         "4k3/8/8/8/8/8/4p3/K7 b - - 0 1", "e2e1r"},
    {"RoqueCurto_Branco",      ROQ_W, "e1g1"},
    {"RoqueLongo_Branco",      ROQ_W, "e1c1"},
    {"RoqueCurto_Preto",       ROQ_B, "e8g8"},
    {"RoqueLongo_Preto",       ROQ_B, "e8c8"},
    {"Rei_PerdeDireitos",      ROQ_W, "e1e2"},
    {"Torre_CapturaTorre",     ROQ_W, "h1h8"},
};
INSTANTIATE_TEST_SUITE_P(Caminhos, CC_Lances_RoundTrip, ::testing::ValuesIn(kRoundTrip),
    [](const ::testing::TestParamInfo<CasoLance>& i) { return std::string(i.param.nome); });

// ---------- novos_direitos: todos os cases do switch ----------
struct CasoDireito { const char* nome; const char* fen; const char* uci; int esperado; };
class CC_Lances_Direitos : public ::testing::TestWithParam<CasoDireito> {};

TEST_P(CC_Lances_Direitos, DireitosDeRoqueAposOLance) {
    const CasoDireito& c = GetParam();
    Posicao p = Posicao::deFen(c.fen);
    ASSERT_TRUE(th::aplicar(p.b, c.uci));
    EXPECT_EQ(static_cast<int>(p.b.estado.direitos_roque), c.esperado);
}
// K=1 Q=2 k=4 q=8
static const CasoDireito kDireitos[] = {
    {"CavaloNaoMexe",      START, "g1f3", 15},
    {"ReiBranco_e1",       ROQ_W, "e1e2", 12},
    {"TorreBranca_h1",     ROQ_W, "h1h2", 14},
    {"TorreBranca_a1",     ROQ_W, "a1a2", 13},
    {"ReiPreto_e8",        ROQ_B, "e8e7", 3},
    {"TorrePreta_h8",      ROQ_B, "h8h7", 11},
    {"TorrePreta_a8",      ROQ_B, "a8a7", 7},
    {"CapturaEmH8",        ROQ_W, "h1h8", 10},   // K some (origem h1) e k some (destino h8)
    {"CapturaEmA8",        ROQ_W, "a1a8", 5},
    {"CapturaEmH1",        ROQ_B, "h8h1", 10},
    {"CapturaEmA1",        ROQ_B, "a8a1", 5},
    {"RoqueCurtoBranco",   ROQ_W, "e1g1", 12},
    {"RoqueCurtoPreto",    ROQ_B, "e8g8", 3},
};
INSTANTIATE_TEST_SUITE_P(Caminhos, CC_Lances_Direitos, ::testing::ValuesIn(kDireitos),
    [](const ::testing::TestParamInfo<CasoDireito>& i) { return std::string(i.param.nome); });

// ---------- estado auxiliar ----------
TEST(CC_Lances, Contador50_TresCaminhos) {
    Posicao a = Posicao::deFen("4k3/8/8/8/8/2N5/8/4K3 w - - 5 10");
    ASSERT_TRUE(th::aplicar(a.b, "c3b5"));
    EXPECT_EQ(a.b.estado.contador_50, 6);                    // lance quieto: ++

    Posicao b = Posicao::deFen("4k3/8/8/8/8/8/4P3/4K3 w - - 7 20");
    ASSERT_TRUE(th::aplicar(b.b, "e2e3"));
    EXPECT_EQ(b.b.estado.contador_50, 0);                    // peao zera

    Posicao c = Posicao::deFen("4k3/8/8/3p4/8/2N5/8/4K3 w - - 5 10");
    ASSERT_TRUE(th::aplicar(c.b, "c3d5"));
    EXPECT_EQ(c.b.estado.contador_50, 0);                    // captura (nao peao) zera
}

TEST(CC_Lances, EnPassant_MarcadoEExpira) {
    Posicao p = Posicao::deFen(START);
    ASSERT_TRUE(th::aplicar(p.b, "e2e4"));
    EXPECT_EQ(p.b.estado.en_passant, Quadrado::e3);          // mesmo sem peao adversario ao lado
    ASSERT_TRUE(th::aplicar(p.b, "g8f6"));
    EXPECT_EQ(p.b.estado.en_passant, Quadrado::quadrado_vazio);
}

TEST(CC_Lances, PecaCapturadaFicaNoEstado) {
    Posicao p = Posicao::deFen("4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1");
    ASSERT_TRUE(th::aplicar(p.b, "e4d5"));
    EXPECT_EQ(p.b.estado.capturada, TipoPeca::PEAO);
    ASSERT_TRUE(th::aplicar(p.b, "e8e7"));
    EXPECT_EQ(p.b.estado.capturada, TipoPeca::PECA_VAZIA);   // novo meio-lance limpa
}

TEST(CC_Lances, EnPassant_RemoveOPeaoCertoNosDoisLados) {
    Posicao w = Posicao::deFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
    ASSERT_TRUE(th::aplicar(w.b, "e5d6"));
    EXPECT_EQ(peca_em(w.b, Quadrado::d5).tpeca, TipoPeca::PECA_VAZIA);
    EXPECT_EQ(peca_em(w.b, Quadrado::d6).tpeca, TipoPeca::PEAO);

    Posicao b = Posicao::deFen("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1");
    ASSERT_TRUE(th::aplicar(b.b, "d4e3"));
    EXPECT_EQ(peca_em(b.b, Quadrado::e4).tpeca, TipoPeca::PECA_VAZIA);
}

TEST(CC_Lances, RoqueMoveATorreNosQuatroLados) {
    struct C { const char* fen; const char* uci; Quadrado de; Quadrado para; };
    const C casos[] = {
        {ROQ_W, "e1g1", Quadrado::h1, Quadrado::f1}, {ROQ_W, "e1c1", Quadrado::a1, Quadrado::d1},
        {ROQ_B, "e8g8", Quadrado::h8, Quadrado::f8}, {ROQ_B, "e8c8", Quadrado::a8, Quadrado::d8}};
    for (const C& c : casos) {
        Posicao p = Posicao::deFen(c.fen);
        ASSERT_TRUE(th::aplicar(p.b, c.uci)) << c.uci;
        EXPECT_EQ(peca_em(p.b, c.de).tpeca, TipoPeca::PECA_VAZIA) << c.uci;
        EXPECT_EQ(peca_em(p.b, c.para).tpeca, TipoPeca::TORRE) << c.uci;
    }
}

// ---------- lances ilegais: false e tabuleiro intacto ----------
TEST(CC_Lances, LanceIlegal_RetornaFalseERestauraTabuleiro) {
    struct C { const char* fen; const char* uci; };
    const C casos[] = {
        {"4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1", "e2d2"},    // peca cravada
        {"4k3/8/8/8/8/8/r7/4K3 w - - 0 1",    "e1e2"},    // rei entra em ataque
    };
    for (int i = 0; i < 2; ++i) {
        Posicao p = Posicao::deFen(casos[i].fen);
        auto antes = std::make_unique<Tabuleiro>(p.b);
        const Lance m = th::acharLance(p.b, casos[i].uci);
        ASSERT_NE(m, SEM_LANCE);
        EXPECT_FALSE(fazer_lance(p.b, m)) << casos[i].uci;
        EXPECT_TRUE(th::iguais(*antes, p.b)) << casos[i].uci;
    }
}
