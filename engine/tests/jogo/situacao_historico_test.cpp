
#include <initializer_list>
#include "helpers_comuns.hpp"

namespace {
void joga(Jogo& j, std::initializer_list<const char*> lances) {
    for (const char* l : lances) ASSERT_TRUE(j.tentarLance(l)) << l;
}
}

// carregarFen zera o historico: a posicao carregada conta como 1a ocorrencia.
TEST(Situacao_Historico, CarregarFenZeraAContagemDeRepeticoes) {
    Jogo j;
    joga(j, {"g1f3", "g8f6", "f3g1", "f6g8"});             // inicial: 2a vez
    EXPECT_EQ(j.situacao().estado, "em_andamento");

    ASSERT_TRUE(j.carregarFen(xadrez::FEN_INICIAL));       // recomeca do zero
    joga(j, {"g1f3", "g8f6", "f3g1", "f6g8"});             // se o historico ficasse, seria a 3a
    EXPECT_EQ(j.situacao().estado, "em_andamento");
}

// Inverso do antigo "EnPassantNaChaveAdiaUmCicloARepeticao": o en passant que NINGUEM pode
// capturar nao faz parte da posicao (FIDE 9.2.3). Posicao apos 1.e4 reaparece nos
// meios-lances 5 e 9 -> 3a vez no 9.
TEST(Situacao_Historico, EnPassantNaoCapturavelNaoAdiaARepeticao) {
    Jogo j;
    joga(j, {"e2e4", "g8f6", "b1c3", "f6g8", "c3b1", "g8f6", "b1c3", "f6g8"});
    EXPECT_EQ(j.situacao().estado, "em_andamento");
    ASSERT_TRUE(j.tentarLance("c3b1"));                    // meio-lance 9
    EXPECT_EQ(j.situacao().estado, "repeticao");
}