#include <gtest/gtest.h>
#include <string>
#include "xadrez/posicao.hpp"
#include "xadrez/movimentos.hpp"
#include "tabuleiro/tabuleiro.hpp"
#include "lances.hpp"
#include "../../infra-estrutura/movimentos.hpp"   // ListaLances e gerar_lances

using namespace xadrez;

static TipoPeca tipoDaLetra(char c) {
    switch (c) {
        case 'q': return TipoPeca::DAMA;
        case 'r': return TipoPeca::TORRE;
        case 'b': return TipoPeca::BISPO;
        default:  return TipoPeca::CAVALO;
    }
}

// Procura entre os lances pseudo-legais da posição o que corresponde ao texto UCI
// ("e2e4", "e7e8q"), SEM validar se deixa o rei em xeque. Assim os testes recebem
// um Lance com as flags certas (captura, en passant, roque, promoção).
static Lance lance(const Posicao& pos, const char* uci) {
    std::string s = uci;
    int dl = 0, dc = 0, pl = 0, pc = 0;
    textoParaCasa(s.substr(0, 2), dl, dc);
    textoParaCasa(s.substr(2, 2), pl, pc);
    Quadrado de   = criar_quadrado(static_cast<Coluna>(dc), static_cast<Linha>(dl));
    Quadrado para = criar_quadrado(static_cast<Coluna>(pc), static_cast<Linha>(pl));

    ListaLances lista;
    gerar_lances(pos.b, lista);
    for (int i = 0; i < lista.quantidade; ++i) {
        Lance m = lista.lances[i];
        if (origem_do_lance(m) != de || destino_do_lance(m) != para) continue;
        if (s.size() == 5) {
            if (eh_promocao(m) && tipo_da_promocao(m) == tipoDaLetra(s[4])) return m;
        } else if (!eh_promocao(m)) {
            return m;
        }
    }
    ADD_FAILURE() << "lance nao gerado: " << uci;
    return SEM_LANCE;
}

// Aplica o lance na posição do FEN e devolve o FEN resultante.
static std::string depois(const char* fen, const char* uci) {
    Posicao p = Posicao::deFen(fen);
    return aplicarLance(p, lance(p, uci)).paraFen();
}

// ---------- lances comuns ----------

TEST(AplicarLance, PeaoAvancaDuasCasasEMarcaEnPassant) {
    EXPECT_EQ(depois(FEN_INICIAL, "e2e4"),
              "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

TEST(AplicarLance, JogadaDasPretasSomaNumeroDoLance) {
    EXPECT_EQ(depois("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", "e7e5"),
              "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
}

TEST(AplicarLance, LanceDeCavaloIncrementaMeioLancesELimpaEnPassant) {
    EXPECT_EQ(depois("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2", "g1f3"),
              "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
    EXPECT_EQ(depois("rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2", "b8c6"),
              "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3");
}

TEST(AplicarLance, CapturaZeraMeioLances) {
    EXPECT_EQ(depois("4k3/8/8/3p4/4P3/8/8/4K3 w - - 5 10", "e4d5"),
              "4k3/8/8/3P4/8/8/8/4K3 b - - 0 10");
}

TEST(AplicarLance, PosicaoOriginalNaoMuda) {
    Posicao antes = Posicao::inicial();
    Posicao nova = aplicarLance(antes, lance(antes, "e2e4"));
    EXPECT_EQ(antes.paraFen(), FEN_INICIAL);
    EXPECT_EQ(peca_em(nova.b, Quadrado::e4).tpeca, TipoPeca::PEAO);
    EXPECT_EQ(peca_em(nova.b, Quadrado::e4).cor, Cor::BRANCO);
    EXPECT_TRUE(casa_vazia(antes.b, Quadrado::e4));
}

// ---------- en passant ----------

TEST(AplicarLance, EnPassantRemoveOPeaoCapturado) {
    // o peão preto em d5 desaparece, embora o peão branco "pouse" em d6
    EXPECT_EQ(depois("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5d6"),
              "4k3/8/3P4/8/8/8/8/4K3 b - - 0 1");
}

TEST(AplicarLance, EnPassantDasPretas) {
    EXPECT_EQ(depois("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1", "d4e3"),
              "4k3/8/8/8/8/4p3/8/4K3 w - - 0 2");
}

// ---------- promoção ----------

TEST(AplicarLance, PromocaoDasBrancasUsaLetraMaiuscula) {
    EXPECT_EQ(depois("k7/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7e8q"),
              "k3Q3/8/8/8/8/8/8/4K3 b - - 0 1");
    EXPECT_EQ(depois("k7/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7e8n"),
              "k3N3/8/8/8/8/8/8/4K3 b - - 0 1");
}

TEST(AplicarLance, PromocaoDasPretasUsaLetraMinuscula) {
    EXPECT_EQ(depois("4k3/8/8/8/8/8/4p3/K7 b - - 0 1", "e2e1n"),
              "4k3/8/8/8/8/8/8/K3n3 w - - 0 2");
}

TEST(AplicarLance, PromocaoComCaptura) {
    EXPECT_EQ(depois("k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7d8r"),
              "k2R4/8/8/8/8/8/8/4K3 b - - 0 1");
}

// ---------- roque (o lance é o do rei andando duas colunas) ----------

TEST(AplicarLance, RoqueCurtoDasBrancasMoveATorre) {
    EXPECT_EQ(depois("4k3/8/8/8/8/8/8/4K2R w K - 0 1", "e1g1"),
              "4k3/8/8/8/8/8/8/5RK1 b - - 1 1");
}

TEST(AplicarLance, RoqueLongoDasBrancasMoveATorre) {
    EXPECT_EQ(depois("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", "e1c1"),
              "4k3/8/8/8/8/8/8/2KR4 b - - 1 1");
}

TEST(AplicarLance, RoqueLongoDasPretasMoveATorre) {
    EXPECT_EQ(depois("r3k3/8/8/8/8/8/8/4K3 b q - 0 1", "e8c8"),
              "2kr4/8/8/8/8/8/8/4K3 w - - 1 2");
}

TEST(AplicarLance, RoqueCurtoDasPretasMoveATorre) {
    EXPECT_EQ(depois("4k2r/8/8/8/8/8/8/4K3 b k - 0 1", "e8g8"),
              "5rk1/8/8/8/8/8/8/4K3 w - - 1 2");
}

// ---------- direitos de roque ----------

TEST(AplicarLance, ReiMovidoPerdeOsDoisRoques) {
    EXPECT_EQ(depois("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1e2"),
              "4k3/8/8/8/8/8/4K3/R6R b - - 1 1");
}

TEST(AplicarLance, TorreMovidaPerdeSoOSeuLado) {
    EXPECT_EQ(depois("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "h1h2"),
              "4k3/8/8/8/8/8/7R/R3K3 b Q - 1 1");
}

TEST(AplicarLance, TorreCapturadaNaCasaDeOrigemTiraORoqueDoAdversario) {
    // a torre branca h1 captura a torre preta h8: brancas perdem K (saiu de h1), pretas perdem k (chegaram em h8)
    EXPECT_EQ(depois("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "h1h8"),
              "r3k2R/8/8/8/8/8/8/R3K3 b Qq - 0 1");
}