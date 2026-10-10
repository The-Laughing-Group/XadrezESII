#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "xadrez/movimentos.hpp"

using namespace xadrez;

// ---------- auxiliares dos testes ----------

// Lances da peça que está em `casa` (ex.: "d4") na posição descrita pelo FEN.
static std::vector<Movimento> lancesDe(const char* fen, const char* casa) {
    int l = 0, c = 0;
    EXPECT_TRUE(textoParaCasa(casa, l, c));
    return movBasicos(Posicao::deFen(fen), l, c);
}

// Existe um lance de `de` para `para` (com a promoção indicada, se houver)?
static bool temLance(const std::vector<Movimento>& v, const char* de, const char* para, char promocao = 0) {
    int dl = 0, dc = 0, pl = 0, pc = 0;
    textoParaCasa(de, dl, dc);
    textoParaCasa(para, pl, pc);
    for (const Movimento& m : v)
        if (m.deLinha == dl && m.deColuna == dc && m.paraLinha == pl && m.paraColuna == pc && m.promocao == promocao)
            return true;
    return false;
}

// Soma os lances de todas as peças do lado indicado.
static int totalDoLado(const Posicao& pos, bool brancas) {
    int total = 0;
    for (int l = 0; l < 8; ++l)
        for (int c = 0; c < 8; ++c)
            if (pertenceAo(pos.tabuleiro[l][c], brancas))
                total += static_cast<int>(movBasicos(pos, l, c).size());
    return total;
}

// Os FENs de teste usam sempre um rei de cada cor (a leitura do FEN exige isso),
// colocados longe das linhas de ação da peça testada.

// ---------- casos de borda ----------

TEST(MovBasicos, CasaVaziaNaoTemLances) {
    EXPECT_TRUE(lancesDe("4k3/8/8/8/8/8/8/4K3 w - - 0 1", "d4").empty());
}

TEST(MovBasicos, ForaDoTabuleiroNaoTemLances) {
    Posicao p = Posicao::inicial();
    EXPECT_TRUE(movBasicos(p, -1, 0).empty());
    EXPECT_TRUE(movBasicos(p, 0, 8).empty());
}

// ---------- cavalo ----------

TEST(MovBasicos, CavaloNoCentroTemOitoLances) {
    auto v = lancesDe("k7/8/8/8/3N4/8/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 8u);
    EXPECT_TRUE(temLance(v, "d4", "e6"));
    EXPECT_TRUE(temLance(v, "d4", "b3"));
}

TEST(MovBasicos, CavaloNoCantoTemDoisLances) {
    auto v = lancesDe("k7/8/8/8/8/8/8/N6K w - - 0 1", "a1");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "a1", "b3"));
    EXPECT_TRUE(temLance(v, "a1", "c2"));
}

TEST(MovBasicos, CavaloNaoPousaEmPecaPropria) {
    // peões brancos em c6 e e6 ocupam duas das oito casas de destino
    auto v = lancesDe("k7/8/2P1P3/8/3N4/8/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 6u);
    EXPECT_FALSE(temLance(v, "d4", "c6"));
    EXPECT_FALSE(temLance(v, "d4", "e6"));
}

TEST(MovBasicos, CavaloCapturaPecaAdversaria) {
    auto v = lancesDe("k7/8/4p3/8/3N4/8/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 8u);
    EXPECT_TRUE(temLance(v, "d4", "e6"));
}

TEST(MovBasicos, CavaloPulaPecasNoCaminho) {
    // cercado de peões, o cavalo ainda tem os 8 lances
    auto v = lancesDe("k7/8/8/2PPP3/2PNP3/2PPP3/8/7K w - - 0 1", "d4");
    // as 8 casas de destino do cavalo estão livres (os peões ocupam só as vizinhas)
    EXPECT_EQ(v.size(), 8u);
}

// ---------- rei ----------

TEST(MovBasicos, ReiNoCentroTemOitoLances) {
    EXPECT_EQ(lancesDe("k7/8/8/8/4K3/8/8/8 w - - 0 1", "e4").size(), 8u);
}

TEST(MovBasicos, ReiNoCantoTemTresLances) {
    EXPECT_EQ(lancesDe("k7/8/8/8/8/8/8/K7 w - - 0 1", "a1").size(), 3u);
}

TEST(MovBasicos, ReiNaoPousaEmPecaPropria) {
    // peões brancos em e5 e f4 bloqueiam duas casas
    auto v = lancesDe("k7/8/8/4P3/4KP2/8/8/8 w - - 0 1", "e4");
    EXPECT_EQ(v.size(), 6u);
}

// ---------- torre ----------

TEST(MovBasicos, TorreLivreNoCanto) {
    // rei branco em h1 bloqueia a última casa da fileira 1
    auto v = lancesDe("7k/8/8/8/8/8/8/R6K w - - 0 1", "a1");
    EXPECT_EQ(v.size(), 13u);
    EXPECT_TRUE(temLance(v, "a1", "a8"));
    EXPECT_TRUE(temLance(v, "a1", "g1"));
    EXPECT_FALSE(temLance(v, "a1", "h1"));
}

TEST(MovBasicos, TorreParaNaPecaPropriaECapturaAdversaria) {
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

TEST(MovBasicos, BispoNoCentroTemTrezeLances) {
    auto v = lancesDe("k7/8/8/8/3B4/8/8/7K w - - 0 1", "d4");
    EXPECT_EQ(v.size(), 13u);
    EXPECT_TRUE(temLance(v, "d4", "a7"));
    EXPECT_TRUE(temLance(v, "d4", "h8"));
    EXPECT_TRUE(temLance(v, "d4", "a1"));
    EXPECT_TRUE(temLance(v, "d4", "g1"));
}

TEST(MovBasicos, BispoBloqueadoPorPecaPropria) {
    // peões brancos em c3 e e5 fecham duas diagonais
    auto v = lancesDe("k7/8/8/4P3/3B4/2P5/8/7K w - - 0 1", "d4");
    // c5,b6,a7 (3) + e3,f2,g1 (3) = 6
    EXPECT_EQ(v.size(), 6u);
}

// ---------- dama ----------

TEST(MovBasicos, DamaNoCentroTemVinteESeteLances) {
    EXPECT_EQ(lancesDe("k7/8/8/8/3Q4/8/8/7K w - - 0 1", "d4").size(), 27u);
}

// ---------- peão ----------

TEST(MovBasicos, PeaoBrancoNaLinhaInicialAnda1Ou2) {
    auto v = lancesDe("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", "e2");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "e2", "e3"));
    EXPECT_TRUE(temLance(v, "e2", "e4"));
}

TEST(MovBasicos, PeaoPretoAndaParaBaixo) {
    auto v = lancesDe("4k3/4p3/8/8/8/8/8/4K3 b - - 0 1", "e7");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "e7", "e6"));
    EXPECT_TRUE(temLance(v, "e7", "e5"));
}

TEST(MovBasicos, PeaoForaDaLinhaInicialAnda1) {
    auto v = lancesDe("4k3/8/8/8/8/4P3/8/4K3 w - - 0 1", "e3");
    EXPECT_EQ(v.size(), 1u);
    EXPECT_TRUE(temLance(v, "e3", "e4"));
}

TEST(MovBasicos, PeaoBloqueadoNaPrimeiraCasaNaoAnda) {
    // cavalo preto em e3: nem 1 nem 2 casas (não pula peça)
    EXPECT_TRUE(lancesDe("4k3/8/8/8/8/4n3/4P3/4K3 w - - 0 1", "e2").empty());
}

TEST(MovBasicos, PeaoBloqueadoNaSegundaCasaAnda1) {
    auto v = lancesDe("4k3/8/8/8/4n3/8/4P3/4K3 w - - 0 1", "e2");
    EXPECT_EQ(v.size(), 1u);
    EXPECT_TRUE(temLance(v, "e2", "e3"));
}

TEST(MovBasicos, PeaoCapturaNasDiagonais) {
    auto v = lancesDe("4k3/8/8/3p1p2/4P3/8/8/4K3 w - - 0 1", "e4");
    EXPECT_EQ(v.size(), 3u);
    EXPECT_TRUE(temLance(v, "e4", "e5"));
    EXPECT_TRUE(temLance(v, "e4", "d5"));
    EXPECT_TRUE(temLance(v, "e4", "f5"));
}

TEST(MovBasicos, PeaoNaoCapturaParaFrenteNemPecaPropria) {
    // peão preto em e5 bloqueia a frente; peão branco em d5 não pode ser capturado
    auto v = lancesDe("4k3/8/8/3Pp3/4P3/8/8/4K3 w - - 0 1", "e4");
    EXPECT_TRUE(v.empty());
}

TEST(MovBasicos, PeaoNaBordaNaoSaiDoTabuleiro) {
    // peão em a4 com adversário em b5: só tem uma diagonal
    auto v = lancesDe("4k3/8/8/1p6/P7/8/8/4K3 w - - 0 1", "a4");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "a4", "a5"));
    EXPECT_TRUE(temLance(v, "a4", "b5"));
}

TEST(MovBasicos, PromocaoGeraQuatroLances) {
    auto v = lancesDe("k7/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7");
    EXPECT_EQ(v.size(), 4u);
    EXPECT_TRUE(temLance(v, "e7", "e8", 'q'));
    EXPECT_TRUE(temLance(v, "e7", "e8", 'r'));
    EXPECT_TRUE(temLance(v, "e7", "e8", 'b'));
    EXPECT_TRUE(temLance(v, "e7", "e8", 'n'));
}

TEST(MovBasicos, PromocaoComCapturaGeraOitoLances) {
    // e8 vazia (4 promoções) + captura da torre em d8 (4 promoções)
    auto v = lancesDe("k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7");
    EXPECT_EQ(v.size(), 8u);
    EXPECT_TRUE(temLance(v, "e7", "d8", 'q'));
}

TEST(MovBasicos, PromocaoDoPeaoPreto) {
    auto v = lancesDe("4k3/8/8/8/8/8/4p3/K7 b - - 0 1", "e2");
    EXPECT_EQ(v.size(), 4u);
    EXPECT_TRUE(temLance(v, "e2", "e1", 'q'));
}

// ---------- en passant ----------

TEST(MovBasicos, EnPassantEhGerado) {
    // o peão preto acabou de andar d7-d5; o peão branco em e5 pode capturar indo para d6
    auto v = lancesDe("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5");
    EXPECT_EQ(v.size(), 2u);
    EXPECT_TRUE(temLance(v, "e5", "e6"));
    EXPECT_TRUE(temLance(v, "e5", "d6"));
}

TEST(MovBasicos, SemCasaDePassantNaoHaCapturaNaCasaVazia) {
    auto v = lancesDe("4k3/8/8/3pP3/8/8/8/4K3 w - - 0 1", "e5");
    EXPECT_EQ(v.size(), 1u);
    EXPECT_TRUE(temLance(v, "e5", "e6"));
}

TEST(MovBasicos, EnPassantSoValePraQuemTemAVez) {
    // a casa d6 é alvo das brancas (é a vez delas); o peão preto em c7 não pode usá-la
    auto v = lancesDe("4k3/2p5/8/3pP3/8/8/8/4K3 w - d6 0 1", "c7");
    EXPECT_EQ(v.size(), 2u);   // só c6 e c5
    EXPECT_FALSE(temLance(v, "c7", "d6"));
}

// ---------- posição inicial ----------

TEST(MovBasicos, PosicaoInicialTemVinteLancesParaCadaLado) {
    Posicao p = Posicao::inicial();
    EXPECT_EQ(totalDoLado(p, true), 20);
    EXPECT_EQ(totalDoLado(p, false), 20);
}
