#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>
#include "xadrez/posicao.hpp"
#include "tabuleiro/tabuleiro.hpp"

using namespace xadrez;

// Convenção nova (LERF): linha 0 = fileira 1, linha 7 = fileira 8.

static void espera(const Posicao& p, Quadrado q, TipoPeca t, Cor c) {
    EXPECT_EQ(peca_em(p.b, q).tpeca, t) << "tipo na casa #" << static_cast<int>(q);
    EXPECT_EQ(peca_em(p.b, q).cor, c) << "cor na casa #" << static_cast<int>(q);
}

static void esperaVazia(const Posicao& p, Quadrado q) {
    EXPECT_EQ(peca_em(p.b, q).tpeca, TipoPeca::PECA_VAZIA) << "casa #" << static_cast<int>(q);
}

// ---------- conversão de casas ----------

TEST(Casas, TextoParaCasa) {
    int l = -1, c = -1;
    ASSERT_TRUE(textoParaCasa("a8", l, c));
    EXPECT_EQ(l, 7); EXPECT_EQ(c, 0);
    ASSERT_TRUE(textoParaCasa("h1", l, c));
    EXPECT_EQ(l, 0); EXPECT_EQ(c, 7);
    ASSERT_TRUE(textoParaCasa("e4", l, c));
    EXPECT_EQ(l, 3); EXPECT_EQ(c, 4);
}

TEST(Casas, CasaParaTexto) {
    EXPECT_EQ(casaParaTexto(0, 0), "a1");
    EXPECT_EQ(casaParaTexto(7, 7), "h8");
    EXPECT_EQ(casaParaTexto(1, 4), "e2");
}

TEST(Casas, IdaEVoltaNasSessentaEQuatroCasas) {
    for (int l = 0; l < 8; ++l)
        for (int c = 0; c < 8; ++c) {
            const std::string texto = casaParaTexto(l, c);
            int l2 = -1, c2 = -1;
            ASSERT_TRUE(textoParaCasa(texto, l2, c2)) << texto;
            EXPECT_EQ(l2, l) << texto;
            EXPECT_EQ(c2, c) << texto;
        }
}

TEST(Casas, TextoInvalido) {
    int l, c;
    EXPECT_FALSE(textoParaCasa("i9", l, c));
    EXPECT_FALSE(textoParaCasa("e", l, c));
    EXPECT_FALSE(textoParaCasa("e44", l, c));
    EXPECT_FALSE(textoParaCasa("", l, c));
    EXPECT_FALSE(textoParaCasa("a0", l, c));     // fileira 0 não existe
    EXPECT_FALSE(textoParaCasa("a9", l, c));     // fileira 9 não existe
    EXPECT_FALSE(textoParaCasa("j1", l, c));     // coluna além de h
    EXPECT_FALSE(textoParaCasa("@1", l, c));     // caractere logo antes de 'a'
    EXPECT_FALSE(textoParaCasa("4e", l, c));     // ordem trocada
    EXPECT_FALSE(textoParaCasa(" e4", l, c));
    EXPECT_FALSE(textoParaCasa("e4 ", l, c));
}

// ---------- FEN: leitura ----------

TEST(Fen, PosicaoInicialNoTabuleiro) {
    Posicao p = Posicao::inicial();

    const Quadrado fileira1[] = {Quadrado::a1, Quadrado::b1, Quadrado::c1, Quadrado::d1,
                                 Quadrado::e1, Quadrado::f1, Quadrado::g1, Quadrado::h1};
    const Quadrado fileira8[] = {Quadrado::a8, Quadrado::b8, Quadrado::c8, Quadrado::d8,
                                 Quadrado::e8, Quadrado::f8, Quadrado::g8, Quadrado::h8};
    const TipoPeca fundo[] = {TipoPeca::TORRE, TipoPeca::CAVALO, TipoPeca::BISPO, TipoPeca::DAMA,
                              TipoPeca::REI,   TipoPeca::BISPO,  TipoPeca::CAVALO, TipoPeca::TORRE};
    for (int i = 0; i < 8; ++i) {
        espera(p, fileira1[i], fundo[i], Cor::BRANCO);
        espera(p, fileira8[i], fundo[i], Cor::PRETO);
    }

    const Quadrado fileira2[] = {Quadrado::a2, Quadrado::b2, Quadrado::c2, Quadrado::d2,
                                 Quadrado::e2, Quadrado::f2, Quadrado::g2, Quadrado::h2};
    const Quadrado fileira7[] = {Quadrado::a7, Quadrado::b7, Quadrado::c7, Quadrado::d7,
                                 Quadrado::e7, Quadrado::f7, Quadrado::g7, Quadrado::h7};
    for (int i = 0; i < 8; ++i) {
        espera(p, fileira2[i], TipoPeca::PEAO, Cor::BRANCO);
        espera(p, fileira7[i], TipoPeca::PEAO, Cor::PRETO);
    }

    for (Quadrado q : {Quadrado::a3, Quadrado::h3, Quadrado::d4, Quadrado::e4,
                       Quadrado::d5, Quadrado::e5, Quadrado::a6, Quadrado::h6})
        esperaVazia(p, q);

    EXPECT_EQ(p.b.estado.lado_a_jogar, Cor::BRANCO);
    EXPECT_EQ(p.b.estado.direitos_roque, TODOS_ROQUES);
    EXPECT_EQ(p.b.estado.en_passant, Quadrado::quadrado_vazio);
    EXPECT_EQ(p.b.estado.contador_50, 0);
}

TEST(Fen, CamposDoEstadoSaoLidos) {
    Posicao p = Posicao::deFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 37 90");
    EXPECT_EQ(p.b.estado.lado_a_jogar, Cor::PRETO);
    EXPECT_EQ(p.b.estado.en_passant, Quadrado::e3);
    EXPECT_EQ(p.b.estado.contador_50, 37);
    EXPECT_EQ(p.paraFen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 37 90");
}

TEST(Fen, IdaEVolta) {
    const char* fens[] = {
        FEN_INICIAL,
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        // posições de referência do perft ("Kiwipete" e as posições 3 a 6)
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        // direitos de roque parciais, en passant das pretas, contadores altos
        "r3k2r/8/8/8/8/8/8/R3K2R w Kq - 0 1",
        "r3k2r/8/8/8/8/8/8/R3K2R b Qk - 12 40",
        "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 25",
        "4k3/8/8/3pP3/8/8/8/4K3 w - d6 99 120",
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1",
    };
    for (const char* fen : fens)
        EXPECT_EQ(Posicao::deFen(fen).paraFen(), fen);
}

TEST(Fen, ComCamposOpcionaisAusentes) {
    Posicao p = Posicao::deFen("4k3/8/8/8/8/8/8/4K3 w - -");
    EXPECT_EQ(p.b.estado.contador_50, 0);
    EXPECT_EQ(p.paraFen(), "4k3/8/8/8/8/8/8/4K3 w - - 0 1");
}

TEST(Fen, ComSoOFullmoveAusente) {
    Posicao p = Posicao::deFen("4k3/8/8/8/8/8/8/4K3 b - - 5");
    EXPECT_EQ(p.b.estado.contador_50, 5);
    EXPECT_EQ(p.paraFen(), "4k3/8/8/8/8/8/8/4K3 b - - 5 1");
}

// Decisão de projeto: um FEN com direito de roque sem o rei ou a torre no lugar
// é aceito e o direito é descartado (a alternativa seria lançar exceção; se for o
// caso, troque este teste por um EXPECT_THROW).
TEST(Fen, DireitosDeRoqueSemReiOuTorreNoLugarSaoDescartados) {
    EXPECT_EQ(Posicao::deFen("4k3/8/8/8/8/8/8/4K3 w KQkq - 0 1").paraFen(),
              "4k3/8/8/8/8/8/8/4K3 w - - 0 1");                       // sem nenhuma torre
    EXPECT_EQ(Posicao::deFen("r3k3/8/8/8/8/8/8/R3K3 w KQq - 0 1").paraFen(),
              "r3k3/8/8/8/8/8/8/R3K3 w Qq - 0 1");                    // só falta a torre h1
    EXPECT_EQ(Posicao::deFen("4k3/8/8/8/8/8/8/R2K3R w KQ - 0 1").paraFen(),
              "4k3/8/8/8/8/8/8/R2K3R w - - 0 1");                     // rei fora de e1
}

// ---------- FEN: entradas inválidas ----------

struct FenInvalido {
    const char* nome;
    const char* fen;
};

static std::string nomeDoFenInvalido(const ::testing::TestParamInfo<FenInvalido>& info) {
    return info.param.nome;
}

class FenInvalidoTest : public ::testing::TestWithParam<FenInvalido> {};

TEST_P(FenInvalidoTest, LancaInvalidArgument) {
    EXPECT_THROW(Posicao::deFen(GetParam().fen), std::invalid_argument) << GetParam().fen;
}

// Erros de forma: não há leitura possível.
static const std::vector<FenInvalido> kFensMalFormados = {
    {"Vazio", ""},
    {"FaltamFileiras", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1"},
    {"FileirasDemais", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/8 w KQkq - 0 1"},
    {"FileiraComNoveCasasPorDigito", "rnbqkbnr/pppppppp/9/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
    {"FileiraComNoveCasasPorLetras", "rnbqkbnrr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
    {"FileiraComSeteCasas", "rnbqkbn/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
    {"FileiraSomaNoveComDigitos", "4k3/8/8/8/8/8/8/5K21 w - - 0 1"},
    {"PecaDesconhecida", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNX w KQkq - 0 1"},
    {"LadoInvalido", "4k3/8/8/8/8/8/8/4K3 x - - 0 1"},
    {"RoqueInvalido", "4k3/8/8/8/8/8/8/4K3 w Z - 0 1"},
    {"EnPassantForaDoTabuleiro", "4k3/8/8/8/8/8/8/4K3 w - e9 0 1"},
    {"EnPassantMalFormado", "4k3/8/8/8/8/8/8/4K3 w - e 0 1"},
    {"MeioLancesNaoNumerico", "4k3/8/8/8/8/8/8/4K3 w - - x 1"},
    {"MeioLancesNegativo", "4k3/8/8/8/8/8/8/4K3 w - - -1 1"},
    {"CamposDemais", "4k3/8/8/8/8/8/8/4K3 w - - 0 1 extra"},
};
INSTANTIATE_TEST_SUITE_P(MalFormados, FenInvalidoTest,
                         ::testing::ValuesIn(kFensMalFormados), nomeDoFenInvalido);

// Erros de significado: o texto é lido, mas a posição não existe numa partida.
// Estas validações vão além da sintaxe; se a decisão de projeto for aceitar
// alguma delas, remova o caso correspondente em vez de enfraquecer o teste.
static const std::vector<FenInvalido> kFensImpossiveis = {
    {"SemReis", "8/8/8/8/8/8/8/8 w - - 0 1"},
    {"SemReiPreto", "8/8/8/8/8/8/8/4K3 w - - 0 1"},
    {"SemReiBranco", "4k3/8/8/8/8/8/8/8 w - - 0 1"},
    {"DoisReisBrancos", "4k3/8/8/8/8/8/8/3KK3 w - - 0 1"},
    {"DoisReisPretos", "3kk3/8/8/8/8/8/8/4K3 w - - 0 1"},
    {"ReisAdjacentes", "8/8/8/8/8/8/4k3/4K3 w - - 0 1"},
    {"PeaoBrancoNaPrimeiraFileira", "4k3/8/8/8/8/8/8/P3K3 w - - 0 1"},
    {"PeaoPretoNaOitavaFileira", "p3k3/8/8/8/8/8/8/4K3 w - - 0 1"},
    {"PeaoBrancoNaOitavaFileira", "P3k3/8/8/8/8/8/8/4K3 w - - 0 1"},
    {"LadoSemAVezEmXeque", "4k3/8/8/8/8/8/8/4R1K1 w - - 0 1"},      // rei preto atacado com as brancas a jogar
    {"EnPassantNaSextaFileiraComPretasAJogar", "4k3/8/8/8/8/8/8/4K3 b - e6 0 1"},
    {"EnPassantNaTerceiraFileiraComBrancasAJogar", "4k3/8/8/8/8/4P3/8/4K3 w - e3 0 1"},
};
INSTANTIATE_TEST_SUITE_P(Impossiveis, FenInvalidoTest,
                         ::testing::ValuesIn(kFensImpossiveis), nomeDoFenInvalido);