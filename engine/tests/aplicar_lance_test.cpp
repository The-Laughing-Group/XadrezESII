#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include "xadrez/posicao.hpp"
#include "xadrez/movimentos.hpp"
#include "tabuleiro/tabuleiro.hpp"
#include "lances.hpp"
#include "../../infra-estrutura/movimentos.hpp"   // ListaLances e gerar_lances
// (o include relativo acima é frágil: prefira target_include_directories no CMake)

using namespace xadrez;

static TipoPeca tipoDaLetra(char c) {
    switch (c) {
        case 'q': return TipoPeca::DAMA;
        case 'r': return TipoPeca::TORRE;
        case 'b': return TipoPeca::BISPO;
        case 'n': return TipoPeca::CAVALO;
        default:  return TipoPeca::PECA_VAZIA;   // letra inválida: nenhum lance vai casar
    }
}

// Procura entre os lances pseudo-legais da posição o que corresponde ao texto UCI
// ("e2e4", "e7e8q"), SEM validar se deixa o rei em xeque. Assim os testes recebem
// um Lance com as flags certas (captura, en passant, roque, promoção).
// Se o lance não existir, registra a falha e devolve SEM_LANCE.
static Lance lance(const Posicao& pos, const char* uci) {
    const std::string s = uci;
    int dl = 0, dc = 0, pl = 0, pc = 0;
    if (s.size() < 4 || !textoParaCasa(s.substr(0, 2), dl, dc) ||
        !textoParaCasa(s.substr(2, 2), pl, pc)) {
        ADD_FAILURE() << "texto UCI invalido no teste: " << uci;
        return SEM_LANCE;
    }
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
    ADD_FAILURE() << "lance nao gerado: " << uci << " em " << pos.paraFen();
    return SEM_LANCE;
}

// Aplica o lance na posição do FEN e devolve o FEN resultante.
// Se o lance não for gerado, devolve um texto que nunca bate com a expectativa
// (em vez de chamar aplicarLance com SEM_LANCE).
static std::string depois(const char* fen, const char* uci) {
    Posicao p = Posicao::deFen(fen);
    const Lance m = lance(p, uci);
    if (m == SEM_LANCE) return "<lance nao gerado>";
    return aplicarLance(p, m).paraFen();
}

// Aplica uma sequência de lances UCI separados por espaço.
static std::string depoisDeVarios(const char* fen, const char* sequencia) {
    Posicao p = Posicao::deFen(fen);
    std::istringstream in(sequencia);
    for (std::string uci; in >> uci;) {
        const Lance m = lance(p, uci.c_str());
        if (m == SEM_LANCE) return "<lance nao gerado: " + uci + ">";
        p = aplicarLance(p, m);
    }
    return p.paraFen();
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

TEST(AplicarLance, LanceDePeaoZeraMeioLancesMesmoSemCaptura) {
    // o contador parte de 7: se o código só "não incrementar", este teste pega
    EXPECT_EQ(depois("4k3/8/8/8/8/8/4P3/4K3 w - - 7 20", "e2e3"),
              "4k3/8/8/8/8/4P3/8/4K3 b - - 0 20");
}

TEST(AplicarLance, FullmoveSoAumentaDepoisDaJogadaDasPretas) {
    EXPECT_EQ(depoisDeVarios(FEN_INICIAL, "e2e4"),           // brancas: continua em 1
              "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    EXPECT_EQ(depoisDeVarios(FEN_INICIAL, "e2e4 e7e5"),      // pretas: vira 2
              "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
}

TEST(AplicarLance, PosicaoOriginalNaoMuda) {
    Posicao antes = Posicao::inicial();
    Posicao nova = aplicarLance(antes, lance(antes, "e2e4"));
    EXPECT_EQ(antes.paraFen(), FEN_INICIAL);
    EXPECT_EQ(peca_em(nova.b, Quadrado::e4).tpeca, TipoPeca::PEAO);
    EXPECT_EQ(peca_em(nova.b, Quadrado::e4).cor, Cor::BRANCO);
    EXPECT_EQ(peca_em(antes.b, Quadrado::e4).tpeca, TipoPeca::PECA_VAZIA);
}

TEST(AplicarLance, PosicaoOriginalNaoMudaEmRoqueEEnPassant) {
    const char* fenRoque = "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1";
    Posicao roque = Posicao::deFen(fenRoque);
    (void)aplicarLance(roque, lance(roque, "e1g1"));
    EXPECT_EQ(roque.paraFen(), fenRoque);

    const char* fenEp = "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1";
    Posicao ep = Posicao::deFen(fenEp);
    (void)aplicarLance(ep, lance(ep, "e5d6"));
    EXPECT_EQ(ep.paraFen(), fenEp);
}

TEST(AplicarLance, SequenciaLongaComRoqueEFimNoFenEsperado) {
    // Ruy Lopez: 1.e4 e5 2.Cf3 Cc6 3.Bb5 a6 4.Ba4 Cf6 5.O-O Be7
    EXPECT_EQ(depoisDeVarios(FEN_INICIAL, "e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7"),
              "r1bqk2r/1pppbppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQ1RK1 w kq - 4 6");
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

TEST(AplicarLance, EnPassantEmPartidaCompleta) {
    EXPECT_EQ(depoisDeVarios(FEN_INICIAL, "e2e4 a7a6 e4e5 d7d5 e5d6"),
              "rnbqkbnr/1pp1pppp/p2P4/8/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3");
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

TEST(AplicarLance, PromocaoDasPretasComCaptura) {
    EXPECT_EQ(depois("4k3/8/8/8/8/8/4p3/K2R4 b - - 0 1", "e2d1q"),
              "4k3/8/8/8/8/8/8/K2q4 w - - 0 2");
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

TEST(AplicarLance, ReiPretoMovidoPerdeOsDoisRoques) {
    EXPECT_EQ(depois("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", "e8e7"),
              "r6r/4k3/8/8/8/8/8/4K3 w - - 1 2");
}

TEST(AplicarLance, TorreMovidaPerdeSoOSeuLado) {
    EXPECT_EQ(depois("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "h1h2"),
              "4k3/8/8/8/8/8/7R/R3K3 b Q - 1 1");
    EXPECT_EQ(depois("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "a1a2"),
              "4k3/8/8/8/8/8/R7/4K2R b K - 1 1");
}

TEST(AplicarLance, TorrePretaMovidaPerdeSoOSeuLado) {
    EXPECT_EQ(depois("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", "a8a7"),
              "4k2r/r7/8/8/8/8/8/4K3 w k - 1 2");
}

TEST(AplicarLance, MoverATorreQueSaiuDoCantoMaisTardeNaoDevolveORoque) {
    // Th1-h2-h1: a torre volta ao canto, mas o direito K já foi perdido
    EXPECT_EQ(depoisDeVarios("r3k3/8/8/8/8/8/8/R3K2R w KQq - 0 1", "h1h2 e8d8 h2h1"),
              "r2k4/8/8/8/8/8/8/R3K2R b Q - 3 2");
}

// Capturar uma torre no canto tira o direito de roque de quem a perdeu.
// Os testes abaixo capturam com BISPO/PEÃO (não com torre) para isolar a regra
// "torre capturada" da regra "torre movida".

TEST(AplicarLance, TorreCapturadaEmH8TiraORoqueCurtoDasPretas) {
    // Bxh8: as brancas não tinham direitos; as pretas perdem k e ficam só com q
    EXPECT_EQ(depois("r3k2r/8/8/8/8/8/8/B3K3 w kq - 0 1", "a1h8"),
              "r3k2B/8/8/8/8/8/8/4K3 b q - 0 1");
}

TEST(AplicarLance, TorreCapturadaEmA8TiraORoqueLongoDasPretas) {
    EXPECT_EQ(depois("r3k2r/8/8/8/8/8/8/4K2B w kq - 0 1", "h1a8"),
              "B3k2r/8/8/8/8/8/8/4K3 b k - 0 1");
}

TEST(AplicarLance, TorreBrancaCapturadaEmA1TiraORoqueLongoDasBrancas) {
    // Bxa1 das pretas: as brancas perdem Q e ficam com K
    EXPECT_EQ(depois("4k2b/8/8/8/8/8/8/R3K2R b KQ - 0 1", "h8a1"),
              "4k3/8/8/8/8/8/8/b3K2R w K - 0 2");
}

TEST(AplicarLance, PromocaoComCapturaDeTorreNoCantoTiraORoque) {
    EXPECT_EQ(depois("r3k2r/6P1/8/8/8/8/8/4K3 w kq - 0 1", "g7h8q"),
              "r3k2Q/8/8/8/8/8/8/4K3 b q - 0 1");
}

TEST(AplicarLance, TorreCapturadaNaCasaDeOrigemTiraORoqueDoAdversario) {
    // caso conjunto: a torre branca h1 captura a torre preta h8.
    // Brancas perdem K (a torre saiu de h1); pretas perdem k (a torre foi capturada em h8).
    EXPECT_EQ(depois("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "h1h8"),
              "r3k2R/8/8/8/8/8/8/R3K3 b Qq - 0 1");
}