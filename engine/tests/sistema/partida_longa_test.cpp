// engine/tests/sistema/partida_longa_test.cpp
// TESTE DE SISTEMA: usa so a API publica do Jogo e atravessa a engine inteira
// (Jogo -> geracao -> fazer_lance -> historico). Rode tambem com ASan/UBSan.
//
// Politica assumida: ao chegar perto do limite do historico o Jogo RECUSA o lance e
// situacao().terminou fica true. O Jogo NAO trava lances em repeticao / regra dos 50,
// entao o ciclo de cavalos abaixo chega ao limite.
#include <string>
#include "helpers_comuns.hpp"
#include "utils/constantes.hpp"    // TAMANHO_HISTORICO
#include "xadrez/posicao.hpp"      // FEN_INICIAL

namespace {
constexpr int MARGEM = 256;   // folga aceitavel reservada antes do limite

// Joga ida e volta de cavalos ate o Jogo recusar. Devolve quantos lances foram aceitos.
int jogarAteRecusar(Jogo& j, bool& recusou) {
    static const char* const CICLO[] = {"g1f3", "g8f6", "f3g1", "f6g8"};
    int aceitos = 0;
    recusou = false;
    for (int i = 0; i < TAMANHO_HISTORICO + 64; ++i) {
        if (!j.tentarLance(CICLO[i % 4])) { recusou = true; break; }
        ++aceitos;
    }
    return aceitos;
}
}  // namespace

TEST(Sistema_PartidaLonga, NuncaPassaDoLimiteDoHistorico) {
    Jogo j;
    bool recusou = false;
    const int aceitos = jogarAteRecusar(j, recusou);

    EXPECT_TRUE(recusou) << "o Jogo aceitou lances alem do limite do historico";
    EXPECT_LE(aceitos, TAMANHO_HISTORICO);
    EXPECT_GE(aceitos, TAMANHO_HISTORICO - MARGEM);
    // (neste cenario repeticao/regra dos 50 tambem marcam terminou; o discriminante
    //  de verdade e o "recusou" acima e o teste unitario CC_Lances_Historico)
    EXPECT_TRUE(j.situacao().terminou);

    // continua seguro depois do limite: consultas e novas tentativas nao quebram
    EXPECT_FALSE(j.tentarLance("g1f3"));
    EXPECT_FALSE(j.tentarLance("e2e4"));
    EXPECT_FALSE(j.fen().empty());
    EXPECT_EQ(j.tabuleiro().size(), 8u);
}

TEST(Sistema_PartidaLonga, CarregarFenDepoisDoLimiteLiberaANovaPartida) {
    Jogo j;
    bool recusou = false;
    jogarAteRecusar(j, recusou);
    ASSERT_TRUE(recusou);

    ASSERT_TRUE(j.carregarFen(xadrez::FEN_INICIAL));
    EXPECT_TRUE(j.tentarLance("e2e4"));
    EXPECT_TRUE(j.tentarLance("e7e5"));
    EXPECT_EQ(j.situacao().estado, "em_andamento");
}