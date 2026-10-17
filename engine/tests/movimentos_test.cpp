#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace xadrez;

// ---------- auxiliares dos testes ----------
// Os testes passam por Jogo::lancesDe, que usa gerar_lances + fazer_lance
// (legalidade). Os lances voltam em texto UCI ("e2e4", "e7e8q").

static std::vector<std::string> lancesDe(const char* fen, const char* casa) {
    Jogo j;
    EXPECT_TRUE(j.carregarFen(fen));
    return j.lancesDe(casa);
}

static bool contem(const std::vector<std::string>& v, const std::string& item) {
    return std::find(v.begin(), v.end(), item) != v.end();
}

// Existe um lance de `de` para `para` (com a promoção indicada, se houver)?
static bool temLance(const std::vector<std::string>& v, const char* de, const char* para, char promocao = 0) {
    std::string uci = std::string(de) + para;
    if (promocao) uci += promocao;
    return contem(v, uci);
}

// Soma os lances de todas as peças do lado que tem a vez no FEN.
static int totalDoLado(const char* fen) {
    Jogo j;
    EXPECT_TRUE(j.carregarFen(fen));
    int total = 0;
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r)
            total += static_cast<int>(j.lancesDe(std::string{c, r}).size());
    return total;
}

// Os FENs de teste usam sempre um rei de cada cor (a leitura do FEN exige isso),
// colocados longe das linhas de ação da peça testada.

// ---------- casos de borda ----------

TEST(movimentos, CasaVaziaNaoTemLances) {
    EXPECT_TRUE(lancesDe("4k3/8/8/8/8/8/8/4K3 w - - 0 1", "d4").empty());
}

TEST(movimentos, ForaDoTabuleiroNaoTemLances) {
    Jogo j;
    EXPECT_TRUE(j.lancesDe("a0").empty());
    EXPECT_TRUE(j.lancesDe("i1").empty());
    EXPECT_TRUE(j.lancesDe("").empty());
}

// ---------- cavalo ----------

TEST(movimentos, CavaloNoCentroTemOitoLances) {
    auto v = lancesDe("k7/8/8/8/3N4/8/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 8u);
    EXPECT_TRUE(temLance(v, "d4", "e6"));
    EXPECT_TRUE(temLance(v, "d4", "b3"));
}

TEST(movimentos, CavaloNoCantoTemDoisLances) {
    auto v = lancesDe("k7/8/8/8/8/8/8/N6K w - - 0 1", "a1");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "a1", "b3"));
    EXPECT_TRUE(temLance(v, "a1", "c2"));
}

TEST(movimentos, CavaloNaoPousaEmPecaPropria) {
    // peões brancos em c6 e e6 ocupam duas das oito casas de destino
    auto v = lancesDe("k7/8/2P1P3/8/3N4/8/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 6u);
    EXPECT_FALSE(temLance(v, "d4", "c6"));
    EXPECT_FALSE(temLance(v, "d4", "e6"));
}

TEST(movimentos, CavaloCapturaPecaAdversaria) {
    auto v = lancesDe("k7/8/4p3/8/3N4/8/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 8u);
    EXPECT_TRUE(temLance(v, "d4", "e6"));
}

TEST(movimentos, CavaloPulaPecasNoCaminho) {
    // cercado de peões, o cavalo ainda tem os 8 lances
    auto v = lancesDe("k7/8/8/2PPP3/2PNP3/2PPP3/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 8u);
}

// ---------- rei ----------

TEST(movimentos, ReiNoCentroTemOitoLances) {
    EXPECT_EQ(lancesDe("k7/8/8/8/4K3/8/8/8 w - - 0 1", "e4").size(), 8u);
}

TEST(movimentos, ReiNoCantoTemTresLances) {
    EXPECT_EQ(lancesDe("k7/8/8/8/8/8/8/K7 w - - 0 1", "a1").size(), 3u);
}

TEST(movimentos, ReiNaoPousaEmPecaPropria) {
    // peões brancos em e5 e f4 bloqueiam duas casas
    auto v = lancesDe("k7/8/8/4P3/4KP2/8/8/8 w - - 0 1", "e4");
    EXPECT_EQ(v.size(), 6u);
}

// ---------- torre ----------

TEST(movimentos, TorreLivreNoCanto) {
    // rei branco em h1 bloqueia a última casa da fileira 1
    auto v = lancesDe("7k/8/8/8/8/8/8/R6K w - - 0 1", "a1");
    EXPECT_EQ(v.size(), 13u);
    EXPECT_TRUE(temLance(v, "a1", "a8"));
    EXPECT_TRUE(temLance(v, "a1", "g1"));
    EXPECT_FALSE(temLance(v, "a1", "h1"));
}

TEST(movimentos, TorreParaNaPecaPropriaECapturaAdversaria) {
    // peão preto em d7 (captura), peão branco em d2 (bloqueio)
    auto v = lancesDe("7k/3p4/8/8/3R4/8/3P4/7K w - - 0 1", "d4");
    // cima: d5, d6, d7(captura) = 3 | baixo: d3 = 1 | esquerda: 3 | direita: 4
    EXPECT_EQ(v.size(), 11u);
    EXPECT_TRUE(temLance(v, "d4", "d7"));
    EXPECT_FALSE(temLance(v, "d4", "d8"));   // não atravessa a peça capturada
    EXPECT_TRUE(temLance(v, "d4", "d3"));
    EXPECT_FALSE(temLance(v, "d4", "d2"));   // peça própria
}

// ---------- bispo ----------

TEST(movimentos, BispoNoCentroTemTrezeLances) {
    auto v = lancesDe("k7/8/8/8/3B4/8/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 13u);
    EXPECT_TRUE(temLance(v, "d4", "a7"));
    EXPECT_TRUE(temLance(v, "d4", "h8"));
    EXPECT_TRUE(temLance(v, "d4", "a1"));
    EXPECT_TRUE(temLance(v, "d4", "g1"));
}

TEST(movimentos, BispoBloqueadoPorPecaPropria) {
    // peões brancos em c3 e e5 fecham duas diagonais
    auto v = lancesDe("k7/8/8/4P3/3B4/2P5/8/7K w - - 0 1", "d4");
    // c5,b6,a7 (3) + e3,f2,g1 (3) = 6
    EXPECT_EQ(v.size(), 6u);
}

// ---------- dama ----------

TEST(movimentos, DamaNoCentroTemVinteESeteLances) {
    EXPECT_EQ(lancesDe("k7/8/8/8/3Q4/8/8/7K w - - 0 1", "d4").size(), 27u);
}

// ---------- peão ----------

TEST(movimentos, PeaoBrancoNaLinhaInicialAnda1Ou2) {
    auto v = lancesDe("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", "e2");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "e2", "e3"));
    EXPECT_TRUE(temLance(v, "e2", "e4"));
}

TEST(movimentos, PeaoPretoAndaParaBaixo) {
    auto v = lancesDe("4k3/4p3/8/8/8/8/8/4K3 b - - 0 1", "e7");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "e7", "e6"));
    EXPECT_TRUE(temLance(v, "e7", "e5"));
}

TEST(movimentos, PeaoForaDaLinhaInicialAnda1) {
    auto v = lancesDe("4k3/8/8/8/8/4P3/8/4K3 w - - 0 1", "e3");
    EXPECT_EQ(v.size(), 1u);
    EXPECT_TRUE(temLance(v, "e3", "e4"));
}

TEST(movimentos, PeaoBloqueadoNaPrimeiraCasaNaoAnda) {
    // cavalo preto em e3: nem 1 nem 2 casas (não pula peça)
    EXPECT_TRUE(lancesDe("4k3/8/8/8/8/4n3/4P3/4K3 w - - 0 1", "e2").empty());
}

TEST(movimentos, PeaoBloqueadoNaSegundaCasaAnda1) {
    auto v = lancesDe("4k3/8/8/8/4n3/8/4P3/4K3 w - - 0 1", "e2");
    EXPECT_EQ(v.size(), 1u);
    EXPECT_TRUE(temLance(v, "e2", "e3"));
}

TEST(movimentos, PeaoCapturaNasDiagonais) {
    auto v = lancesDe("4k3/8/8/3p1p2/4P3/8/8/4K3 w - - 0 1", "e4");
    EXPECT_EQ(v.size(), 3u);
    EXPECT_TRUE(temLance(v, "e4", "e5"));
    EXPECT_TRUE(temLance(v, "e4", "d5"));
    EXPECT_TRUE(temLance(v, "e4", "f5"));
}

TEST(movimentos, PeaoNaoCapturaParaFrenteNemPecaPropria) {
    // peão preto em e5 bloqueia a frente; peão branco em d5 não pode ser capturado
    auto v = lancesDe("4k3/8/8/3Pp3/4P3/8/8/4K3 w - - 0 1", "e4");
    EXPECT_TRUE(v.empty());
}

TEST(movimentos, PeaoNaBordaNaoSaiDoTabuleiro) {
    // peão em a4 com adversário em b5: só tem uma diagonal
    auto v = lancesDe("4k3/8/8/1p6/P7/8/8/4K3 w - - 0 1", "a4");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "a4", "a5"));
    EXPECT_TRUE(temLance(v, "a4", "b5"));
}

TEST(movimentos, PromocaoGeraQuatroLances) {
    auto v = lancesDe("k7/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7");
    EXPECT_EQ(v.size(), 4u);
    EXPECT_TRUE(temLance(v, "e7", "e8", 'q'));
    EXPECT_TRUE(temLance(v, "e7", "e8", 'r'));
    EXPECT_TRUE(temLance(v, "e7", "e8", 'b'));
    EXPECT_TRUE(temLance(v, "e7", "e8", 'n'));
}

TEST(movimentos, PromocaoComCapturaGeraOitoLances) {
    // e8 vazia (4 promoções) + captura da torre em d8 (4 promoções)
    auto v = lancesDe("k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7");
    EXPECT_EQ(v.size(), 8u);
    EXPECT_TRUE(temLance(v, "e7", "d8", 'q'));
}

TEST(movimentos, PromocaoDoPeaoPreto) {
    auto v = lancesDe("4k3/8/8/8/8/8/4p3/K7 b - - 0 1", "e2");
    EXPECT_EQ(v.size(), 4u);
    EXPECT_TRUE(temLance(v, "e2", "e1", 'q'));
}

// ---------- en passant ----------

TEST(movimentos, EnPassantEhGerado) {
    // o peão preto acabou de andar d7-d5; o peão branco em e5 pode capturar indo para d6
    auto v = lancesDe("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "e5", "e6"));
    EXPECT_TRUE(temLance(v, "e5", "d6"));
}

TEST(movimentos, SemCasaDePassantNaoHaCapturaNaCasaVazia) {
    auto v = lancesDe("4k3/8/8/3pP3/8/8/8/4K3 w - - 0 1", "e5");
    EXPECT_EQ(v.size(), 1u);
    EXPECT_TRUE(temLance(v, "e5", "e6"));
}

TEST(movimentos, EnPassantSoValePraQuemTemAVez) {
    // a casa d6 é alvo das brancas (é a vez delas); o peão preto em c7 não tem a vez.
    // Pelo Jogo, peça de quem não tem a vez não tem lances.
    auto v = lancesDe("4k3/2p5/8/3pP3/8/8/8/4K3 w - d6 0 1", "c7");
    EXPECT_TRUE(v.empty());
}

// ---------- posição inicial ----------

TEST(movimentos, PosicaoInicialTemVinteLancesParaCadaLado) {
    EXPECT_EQ(totalDoLado("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), 20);
    EXPECT_EQ(totalDoLado("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"), 20);
}

// =====================================================================
// ROQUE
// O rei testado tem 5 lances normais quando está em e1/e8 sem peças ao lado
// (d, f e a fileira de cima/baixo); cada roque soma mais 1.
// =====================================================================

// ---------- roque: o lance é gerado ----------

TEST(movimentos, RoqueCurtoDasBrancas) {
    auto v = lancesDe("4k3/8/8/8/8/8/8/4K2R w K - 0 1", "e1");
    EXPECT_TRUE(temLance(v, "e1", "g1"));
    EXPECT_FALSE(temLance(v, "e1", "c1"));     // sem direito de roque longo
    EXPECT_EQ(v.size(), 6u);                    // 5 lances normais + roque
}

TEST(movimentos, RoqueLongoDasBrancas) {
    auto v = lancesDe("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", "e1");
    EXPECT_TRUE(temLance(v, "e1", "c1"));
    EXPECT_FALSE(temLance(v, "e1", "g1"));
    EXPECT_EQ(v.size(), 6u);
}

TEST(movimentos, RoqueDoisRoquesDasBrancas) {
    auto v = lancesDe("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1");
    EXPECT_TRUE(temLance(v, "e1", "g1"));
    EXPECT_TRUE(temLance(v, "e1", "c1"));
    EXPECT_EQ(v.size(), 7u);
}

TEST(movimentos, RoqueDasPretas) {
    auto v = lancesDe("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", "e8");
    EXPECT_TRUE(temLance(v, "e8", "g8"));
    EXPECT_TRUE(temLance(v, "e8", "c8"));
    EXPECT_EQ(v.size(), 7u);
}

TEST(movimentos, RoqueSoUmLadoQuandoSoTemUmDireito) {
    auto v = lancesDe("r3k2r/8/8/8/8/8/8/4K3 b k - 0 1", "e8");
    EXPECT_TRUE(temLance(v, "e8", "g8"));
    EXPECT_FALSE(temLance(v, "e8", "c8"));
}

// ---------- roque: o lance NÃO é gerado ----------

TEST(movimentos, RoqueSemDireitoNaoGera) {
    auto v = lancesDe("4k3/8/8/8/8/8/8/R3K2R w - - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "g1"));
    EXPECT_FALSE(temLance(v, "e1", "c1"));
    EXPECT_EQ(v.size(), 5u);
}

TEST(movimentos, RoqueCurtoBloqueadoPorPecaEntreReiETorre) {
    // cavalo branco em f1
    auto v = lancesDe("4k3/8/8/8/8/8/8/4KN1R w K - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "g1"));
}

TEST(movimentos, RoqueCurtoBloqueadoPorPecaAdversariaEmG1) {
    auto v = lancesDe("4k3/8/8/8/8/8/8/4K1nR w K - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "g1"));
}

TEST(movimentos, RoqueLongoBloqueadoPorPecaEmD1OuC1) {
    EXPECT_FALSE(temLance(lancesDe("4k3/8/8/8/8/8/8/R2BK3 w Q - 0 1", "e1"), "e1", "c1"));
    EXPECT_FALSE(temLance(lancesDe("4k3/8/8/8/8/8/8/R1B1K3 w Q - 0 1", "e1"), "e1", "c1"));
}

TEST(movimentos, RoqueLongoPrecisaDaCasaBLivreMesmoSeOReiNaoPassaPorLa) {
    // bispo em b1: o rei não passa por b1, mas a torre precisa do caminho livre
    auto v = lancesDe("4k3/8/8/8/8/8/8/RB2K3 w Q - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "c1"));
}

// ATENÇÃO: estes dois dependem de gerar_lances (ou carregar_fen) conferir que há
// uma torre da própria cor no canto. Hoje movimentos.cpp só olha o direito de roque
// e as casas vazias; se falharem, o problema é no código, não no teste.
TEST(movimentos, RoqueSemATorreNoCantoNaoGera) {
    // o FEN diz que há direito (K), mas não há torre em h1
    auto v = lancesDe("4k3/8/8/8/8/8/8/4K3 w K - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "g1"));
    EXPECT_EQ(v.size(), 5u);
}

TEST(movimentos, RoqueTorreAdversariaNoCantoNaoServe) {
    auto v = lancesDe("4k3/8/8/8/8/8/8/4K2r w K - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "g1"));
}

// ---------- FENs inconsistentes: não podem gerar roque falso ----------

TEST(movimentos, RoqueReiForaDaCasaInicialNaoFazRoque) {
    // direito K no FEN, mas o rei está em d1: não pode aparecer o "lance de duas casas" d1f1
    auto v = lancesDe("4k3/8/8/8/8/8/8/3K4 w K - 0 1", "d1");
    EXPECT_FALSE(temLance(v, "d1", "f1"));
    EXPECT_EQ(v.size(), 5u);
}

TEST(movimentos, RoqueReiNaColunaHNaoGeraLanceExtra) {
    auto v = lancesDe("4k3/8/8/8/8/8/8/7K w KQ - 0 1", "h1");
    EXPECT_EQ(v.size(), 3u);
}

TEST(movimentos, RoqueReiNaColunaGNaoGeraLanceExtra) {
    auto v = lancesDe("4k3/8/8/8/8/8/8/6K1 w KQ - 0 1", "g1");
    EXPECT_EQ(v.size(), 5u);
}

TEST(movimentos, RoqueReiNaLinhaDeCimaDasBrancasNaoFazRoque) {
    // rei branco em e8 (linha errada para as brancas) com direitos no FEN
    auto v = lancesDe("r3K3/8/8/8/8/8/8/4k3 w KQ - 0 1", "e8");
    EXPECT_FALSE(temLance(v, "e8", "g8"));
    EXPECT_FALSE(temLance(v, "e8", "c8"));
}

// ---------- roque: casa b atacada não atrapalha ----------

TEST(movimentos, RoqueCasaBAtacadaNaoImpedeORoqueLongo) {
    // torre preta em b8 ataca b1; o rei não passa por b1, então o roque longo continua valendo
    auto v = lancesDe("kr6/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1");
    EXPECT_TRUE(temLance(v, "e1", "c1"));
    EXPECT_TRUE(temLance(v, "e1", "g1"));
}

// ---------- roque: executar pelo Jogo (gerar + validar + aplicar) ----------

TEST(movimentos, RoqueNoJogo_ListaOsDoisRoques) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    auto v = j.lancesDe("e1");
    EXPECT_TRUE(contem(v, "e1g1"));
    EXPECT_TRUE(contem(v, "e1c1"));
}

TEST(movimentos, RoqueNoJogo_RoqueCurtoMoveReiETorre) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    EXPECT_TRUE(j.tentarLance("e1g1"));
    EXPECT_EQ(j.fen(), "4k3/8/8/8/8/8/8/R4RK1 b - - 1 1");   // perdeu os dois direitos
}

TEST(movimentos, RoqueNoJogo_RoqueLongoDasPretasMoveReiETorre) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1"));
    EXPECT_TRUE(j.tentarLance("e8c8"));
    EXPECT_EQ(j.fen(), "2kr3r/8/8/8/8/8/8/4K3 w - - 1 2");
}

TEST(movimentos, RoqueNoJogo_TorreMovidaTiraSoOSeuRoque) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    ASSERT_TRUE(j.tentarLance("h1h2"));      // brancas
    ASSERT_TRUE(j.tentarLance("e8d8"));      // pretas
    auto v = j.lancesDe("e1");
    EXPECT_FALSE(contem(v, "e1g1"));         // a torre h já saiu
    EXPECT_TRUE(contem(v, "e1c1"));          // a torre a continua valendo
}

TEST(movimentos, RoqueNoJogo_ReiMovidoNaoVoltaAFazerRoque) {
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

TEST(movimentos, RoqueNoJogo_TorreCapturadaNoCantoTiraORoqueDoDono) {
    Jogo j;
    // a torre preta sai de h8 e captura a torre branca em h1:
    // as brancas perdem o roque curto (K) e as pretas também (k), porque a torre delas saiu de h8
    ASSERT_TRUE(j.carregarFen("4k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1"));
    ASSERT_TRUE(j.tentarLance("h8h1"));
    EXPECT_EQ(j.fen(), "4k3/8/8/8/8/8/8/R3K2r w Q - 0 2");
}

// ---------- roque e casas atacadas (casa_atacada já existe: DISABLED_ removido) ----------

TEST(movimentos, RoqueReiEmXequeNaoFazRoque) {
    // torre preta em e5 dá xeque no rei branco em e1
    auto v = lancesDe("k7/8/8/4r3/8/8/8/R3K2R w KQ - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "g1"));
    EXPECT_FALSE(temLance(v, "e1", "c1"));
}

TEST(movimentos, RoqueReiNaoPassaPorCasaAtacada_Curto) {
    // torre preta em f8 ataca f1 (casa que o rei atravessa); o roque longo continua livre
    auto v = lancesDe("k4r2/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "g1"));
    EXPECT_TRUE(temLance(v, "e1", "c1"));
}

TEST(movimentos, RoqueReiNaoPassaPorCasaAtacada_Longo) {
    // torre preta em d8 ataca d1; o roque curto continua livre
    auto v = lancesDe("k2r4/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "c1"));
    EXPECT_TRUE(temLance(v, "e1", "g1"));
}

TEST(movimentos, RoqueReiNaoTerminaEmCasaAtacada) {
    // torre preta em g8 ataca g1 (destino do roque curto)
    auto v = lancesDe("k5r1/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1");
    EXPECT_FALSE(temLance(v, "e1", "g1"));
    EXPECT_TRUE(temLance(v, "e1", "c1"));
}

TEST(movimentos, RoquePretasTambemRespeitamCasasAtacadas) {
    // torre branca em f1 ataca f8
    auto v = lancesDe("r3k2r/8/8/8/8/8/8/4KR2 b kq - 0 1", "e8");
    EXPECT_FALSE(temLance(v, "e8", "g8"));
    EXPECT_TRUE(temLance(v, "e8", "c8"));
}